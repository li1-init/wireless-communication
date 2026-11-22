/**
 * @file radio_link.c
 * @brief Si4438 无线收发驱动实现。
 */
#include "radio_link.h"
#include "radio_port.h"
#include "radio_config_b1_10k.h"
#include "spidma.h"

/* R2: the real TX returned PART=4438, CHIPREV=11, ROMID=03 and rejected
 * the R1 C2 table at offset 62 (SYNC_CONFIG2, property 1105).
 * Use a complete WDS B1-API modem table instead of dropping arbitrary bytes
 * from the C2 table. The imported Si4464 B1 table is a compatibility starting
 * point for this observed EZRadioPRO API; RF operation still needs board test.
 * Retune 434 -> 433MHz; retain board XO tuning and reduced PA level.
 * Both projects use exactly the same 10kbps / 20kHz deviation configuration. */
/* 清除前面的宏定义以使用当前模块配置。 */
#undef RF_GPIO_PIN_CFG
#define RF_GPIO_PIN_CFG 0x13,1,1,1,1,0,0,0
/* 清除前面的宏定义以使用当前模块配置。 */
#undef RF_GLOBAL_XO_TUNE_2
#define RF_GLOBAL_XO_TUNE_2 0x11,0x00,2,0,0x45,0x00
/* 清除前面的宏定义以使用当前模块配置。 */
#undef RF_FREQ_CONTROL_INTE_8
#define RF_FREQ_CONTROL_INTE_8 0x11,0x40,8,0,0x38,0x0D,0xDD,0xDD,0x44,0x44,0x20,0xFE
/* 清除前面的宏定义以使用当前模块配置。 */
#undef RF_PA_MODE_4
#define RF_PA_MODE_4 0x11,0x22,4,0,0x08,0x10,0x00,0x3D
static const uint8_t configuration[]=RADIO_CONFIGURATION_DATA_ARRAY;

volatile RadioLinkStatus g_radio;

/**
 * @brief 无线收发队列。
 */
typedef struct
{
    PacketRing tx;
    PacketRing rx;
} RadioQueueState;

static RadioQueueState radio_queues;

volatile PacketRingInfo g_radio_rx_buffer;
volatile PacketRingInfo g_radio_tx_buffer;
/**
 * @brief 取得 RX 环形区快照。
 * @param info 输出的队列状态，可为空。
 */
void Radio_GetRxBufferInfo(PacketRingInfo *info)
{
    PacketRingInfo snapshot;
    PacketRing_GetInfo(&radio_queues.rx, &snapshot);
    g_radio_rx_buffer=snapshot;
    if(info) *info=snapshot;
}
/**
 * @brief 取得 TX 环形区快照。
 * @param info 输出的队列状态，可为空。
 */
void Radio_GetTxBufferInfo(PacketRingInfo *info)
{
    PacketRingInfo snapshot;
    PacketRing_GetInfo(&radio_queues.tx, &snapshot);
    g_radio_tx_buffer=snapshot;
    if(info) *info=snapshot;
}
/**
 * @brief 复制最近收到的完整帧。
 * @param packet 输出缓冲区。
 * @param capacity 输出缓冲区容量。
 * @return 复制的数据长度。
 */
uint16_t Radio_CopyLastReceived(uint8_t *packet,uint16_t capacity)
{
    return PacketRing_CopyLast(&radio_queues.rx, packet, capacity);
}

/**
 * @brief 记录首次无线错误并返回失败。
 * @param error 无线驱动错误码。
 * @return 固定返回 0。
 */
static uint8_t Fail(uint8_t error)
{
    if(g_radio.error==RF_OK) g_radio.error=error;
    return 0U;
}
/**
 * @brief 调用板级单字节 SPI。
 * @param tx 待发送字节。
 * @param rx 接收字节输出地址。
 * @return 传输成功时返回 1，否则返回 0。
 */
static uint8_t Byte(uint8_t tx,uint8_t *rx)
{
    return Board_SpiByte(tx,rx) ? 1U : Fail(RF_SPI_TIMEOUT);
}
/**
 * @brief 等待 SPI 发送结束后释放片选。
 * @return 操作成功时返回 1，否则返回 0。
 */
static uint8_t End(void)
{
    uint8_t ok=Board_SpiFinish();
    Board_Nsel(1U);
    return ok ? 1U : Fail(RF_SPI_TIMEOUT);
}
/**
 * @brief 轮询无线 CTS 并读取回复。
 * @param reply 回复数据输出缓冲区。
 * @param length 回复数据长度。
 * @return 操作成功时返回 1，否则返回 0。
 */
static uint8_t Response(uint8_t *reply,uint8_t length)
{
    uint32_t start;
    uint8_t cts;
    uint8_t discard;
    uint8_t index;

    start=Board_Millis();
    do {
        Board_Nsel(0U);
        if(!Byte(0x44U,&discard)||!Byte(0xFFU,&cts)) {
            Board_Nsel(1U); return 0U;
        }
        g_radio.last_cts=cts;
        if(cts==0xFFU) {
            for(index=0;index<length;++index)
                if(!Byte(0xFFU, reply + index))
                {
                    Board_Nsel(1U);
                    return 0U;
                }
            return End();
        }
        if(!End()) return 0U;
        Board_DelayMs(1U);
    } while((uint32_t)(Board_Millis()-start)<500UL);
    return Fail(RF_CTS_TIMEOUT);
}
/**
 * @brief 执行一次无线命令并读取回复。
 * @param cmd 命令数据。
 * @param length 命令长度。
 * @param reply 回复数据输出缓冲区。
 * @param reply_length 回复数据长度。
 * @return 操作成功时返回 1，否则返回 0。
 */
static uint8_t Command(const uint8_t *cmd,uint8_t length,uint8_t *reply,uint8_t reply_length)
{
    uint8_t index;
    uint8_t discard;
    g_radio.last_command=cmd[0];
    if(cmd[0]!=0x23U) {
        g_radio.request_length=length;
        for(index=0;index<16U;++index)
            g_radio.request_bytes[index]=(index<length)?cmd[index]:0U;
    }
    if(!Response(0,0)) return 0U;
    Board_Nsel(0U);
    for(index=0;index<length;++index)
        if(!Byte(cmd[index], &discard))
        {
            Board_Nsel(1U);
            return 0U;
        }
    if(!End()) return 0U;
    return Response(reply,reply_length);
}
/**
 * @brief 读取芯片命令错误状态并转换成驱动错误码。
 * @return 没有芯片错误时返回 1，否则返回 0。
 */
static uint8_t CheckError(void)
{
    const uint8_t cmd[]={0x23,0};
    uint8_t r[4];
    if(!Command(cmd,sizeof(cmd),r,sizeof(r))) return 0U;
    g_radio.command_error=r[2];
    g_radio.command_error_id=r[3];
    if(((r[0]|r[1])&0x08U)||r[2]) return Fail(RF_COMMAND_ERROR);
    return 1U;
}
/**
 * @brief 发送配置命令并检查无线芯片错误状态。
 * @param cmd 配置命令。
 * @param length 命令长度。
 * @return 操作成功时返回 1，否则返回 0。
 */
static uint8_t Checked(const uint8_t *cmd,uint8_t length)
{
    return Command(cmd,length,0,0) && CheckError();
}
/**
 * @brief 读取并清除无线中断状态。
 * @param r 输出的中断状态字节。
 * @return 读取成功时返回 1，否则返回 0。
 */
static uint8_t Interrupts(uint8_t r[8])
{
    const uint8_t cmd[]={0x20,0,0,0};
    if(!Command(cmd,sizeof(cmd),r,8)) return 0U;
    g_radio.last_ph_pending=r[2];
    g_radio.last_chip_pending=r[6];
    if(r[6]&0x08U) {
        (void)CheckError();
        return Fail(RF_COMMAND_ERROR);
    }
    return 1U;
}
/**
 * @brief 查询 FIFO 字节数并按需复位 FIFO。
 * @param reset FIFO 复位选项。
 * @param reply FIFO 状态输出缓冲区。
 * @return 操作成功时返回 1，否则返回 0。
 */
static uint8_t FifoInfo(uint8_t reset,uint8_t reply[2])
{
    const uint8_t cmd[]={0x15,reset};
    return Command(cmd,sizeof(cmd),reply,2U);
}
/**
 * @brief 发送 FIFO 操作码并使用 DMA 搬运数据。
 * @param opcode FIFO 操作码。
 * @param tx 待发送数据，可为空。
 * @param rx 接收数据缓冲区，可为空。
 * @return 操作成功时返回 1，否则返回 0。
 */
static uint8_t FifoTransfer(uint8_t opcode,const uint8_t *tx,uint8_t *rx)
{
    uint8_t b;
    if(!Response(0,0)) return 0U;
    Board_Nsel(0U);
    if(!Byte(opcode,&b)) { Board_Nsel(1U); return 0U; }
    if(!SpiDma_Transfer(tx,rx,LINK_PACKET_SIZE)) {
        Board_Nsel(1U);
        return Fail(RF_DMA_ERROR);
    }
    return End();
}
/**
 * @brief 把无线芯片切换到 READY 并轮询确认状态。
 * @return 芯片进入 READY 时返回 1，否则返回 0。
 */
static uint8_t Ready(void)
{
    const uint8_t cmd[]={0x34,3};
    const uint8_t query=0x33;
    uint8_t reply[2];
    uint32_t start;
    if(!Command(cmd,sizeof(cmd),0,0)) return 0U;
    start=Board_Millis();
    do {
        if(!Command(&query,1,reply,2)) return 0U;
        g_radio.device_state=reply[0];
        if((reply[0]&0x0FU)==3U) return 1U;
        Board_DelayMs(1);
    } while((uint32_t)(Board_Millis()-start)<100UL);
    return Fail(RF_COMMAND_ERROR);
}
/**
 * @brief 读回无线配置属性并逐字节比较。
 * @param group 属性组编号。
 * @param offset 属性偏移。
 * @param expected 期望值。
 * @param length 比较长度。
 * @return 配置匹配时返回 1，否则返回 0。
 */
static uint8_t Verify(uint8_t group,uint8_t offset,const uint8_t *expected,uint8_t length)
{
    uint8_t cmd[]={0x12,group,length,offset};
    uint8_t reply[12];
    uint8_t index;
    if(length>sizeof(reply)) return Fail(RF_CONFIG_ERROR);
    if(!Command(cmd,sizeof(cmd),reply,length)) return 0U;
    for(index=0;index<length;++index)
        if(reply[index] != expected[index])
        {
            return Fail(RF_CONFIG_ERROR);
        }
    return CheckError();
}
/** @brief 配置固定包长、同步字、CRC 和中断，并读回核对关键参数。 */
static uint8_t ConfigurePacket(void)
{
    /* B1 packet handler uses one shared field table. Property 1207
     * (PKT_CONFIG2) and C2's separate RX fields must never be addressed. */
    const uint8_t crc[]={0x11,0x12,1,0,0x85};
    const uint8_t packet[]={0x11,0x12,1,6,0x02};
    const uint8_t fixed_length[]={0x11,0x12,3,8,0,0,0};
    const uint8_t field[]={0x11,0x12,4,0x0D,0,LINK_PACKET_SIZE,0,0xAA};
    const uint8_t field2[]={0x11,0x12,4,0x11,0,0,0,0};
    const uint8_t field3[]={0x11,0x12,4,0x15,0,0,0,0};
    const uint8_t field4[]={0x11,0x12,4,0x19,0,0,0,0};
    const uint8_t field5[]={0x11,0x12,4,0x1D,0,0,0,0};
    const uint8_t interrupts[]={0x11,0x01,4,0,0x05,0x38,0x00,0x28};
    const uint8_t expected_freq[]={0x38,0x0D,0xDD,0xDD,0x44,0x44,0x20,0xFE};
    const uint8_t expected_sync[]={1,0xB4,0x2B};
    const uint8_t expected_mod[]={3,0,7,0x06,0x1A,0x80};
    const uint8_t expected_pa[]={0x08,0x10,0,0x3D};
    if(!Checked(crc,sizeof(crc)) || !Checked(packet,sizeof(packet)) ||
       !Checked(fixed_length,sizeof(fixed_length)) || !Checked(field,sizeof(field)) ||
       !Checked(field2,sizeof(field2)) || !Checked(field3,sizeof(field3)) ||
       !Checked(field4,sizeof(field4)) || !Checked(field5,sizeof(field5)) ||
       !Checked(interrupts,sizeof(interrupts))) return 0U;
    g_radio.stage=4;
    return Verify(0x12,0,crc+4,1) && Verify(0x12,6,packet+4,1) &&
           Verify(0x12,8,fixed_length+4,3) &&
           Verify(0x12,0x0D,field+4,4) && Verify(0x01,0,interrupts+4,4) &&
           Verify(0x40,0,expected_freq,8) && Verify(0x11,0,expected_sync,3) &&
           Verify(0x20,0,expected_mod,6) && Verify(0x22,0,expected_pa,4);
}
/**
 * @brief 释放片选并关闭无线芯片。
 */
void Radio_Shutdown(void)
{
    Board_Nsel(1U);
    Board_Sdn(1U);
}
/**
 * @brief 重置包队列并初始化无线芯片。
 * @return 初始化成功时返回 1，否则返回 0。
 */
uint8_t Radio_Init(void)
{
    uint16_t position;
    uint8_t count;
    uint8_t reply[8];
    uint8_t fifo[2];
    uint8_t index;
    const uint8_t part = 0x01U;

    position = 0U;
    PacketRing_Init(&radio_queues.tx);
    PacketRing_Init(&radio_queues.rx);
    Radio_GetRxBufferInfo(0);
    Radio_GetTxBufferInfo(0);
    ++g_radio.init_count;
    g_radio.error=RF_OK;
    g_radio.command_error = 0U;
    g_radio.command_error_id = 0U;
    g_radio.config_offset = 0U;
    g_radio.profile_version=2U;
    g_radio.device_state=0U;
    g_radio.stage=1;
    Board_RadioInit();
    Board_Sdn(1U);
    Board_DelayMs(10U);
    Board_Sdn(0U);
    Board_DelayMs(20U);
    /* First WDS command is exactly POWER_UP (7 bytes). */
    if(configuration[0]!=7 || configuration[1]!=0x02) {
        Fail(RF_CONFIG_ERROR); goto failed;
    }
    if(!Checked(configuration+1,7)) goto failed;
    position = 8U;
    g_radio.stage=2;
    if(!Command(&part, 1U, reply, 8U)) goto failed;
    for(index=0;index<8U;++index)
        g_radio.part_info[index]=reply[index];
    g_radio.part=(uint16_t)((uint16_t)reply[1]<<8)|reply[2];
    if(g_radio.part!=0x4438U) { Fail(RF_WRONG_PART); goto failed; }
    /* The square TX has 0x11/0x03; the round RX currently on the bench
     * reports 0x22/0x06. Both identify the same 0x4438 family and must be
     * allowed to proceed with the common B1-compatible profile. */
    if(!((reply[0]==0x11U && reply[7]==0x03U) ||
         (reply[0]==0x22U && reply[7]==0x06U))) {
        Fail(RF_UNSUPPORTED_REV); goto failed;
    }
    g_radio.stage=3;
    while(position<sizeof(configuration)) {
        g_radio.config_offset=position;
        count=configuration[position++];
        if(count==0U) break;
        if(count>16U ||
           (uint32_t)position+count>sizeof(configuration)) {
            Fail(RF_CONFIG_ERROR); goto failed;
        }
        if(!Checked(configuration+position,count)) goto failed;
        position=(uint16_t)(position+count);
    }
    if(position!=sizeof(configuration) ||
       configuration[position-1U]!=0U) {
        Fail(RF_CONFIG_ERROR); goto failed;
    }
    if(!ConfigurePacket() ||
       !Ready() ||
       !FifoInfo(3U, fifo) ||
       !Interrupts(reply))
        goto failed;
    g_radio.stage=5;
    return 1U;
failed:
    Radio_Shutdown();
    return 0U;
}
/**
 * @brief 将一帧写入无线 FIFO 并等待发送完成。
 * @param packet 待发送的固定长度帧。
 * @return 发送成功时返回 1，否则返回 0。
 */
static uint8_t SendPacket(const uint8_t packet[LINK_PACKET_SIZE])
{
    const uint8_t start_tx[]={0x31,LINK_CHANNEL,0x30,0,0,0,0};
    uint8_t irq[8];
    uint8_t fifo[2];
    uint32_t started;
    if(g_radio.error!=RF_OK) return 0U;
    g_radio.stage=6;
    if(!Ready() || !Interrupts(irq) || !FifoInfo(1,fifo)) goto failed;
    if(fifo[1]<LINK_PACKET_SIZE) { Fail(RF_FIFO_ERROR); goto failed; }
    if(!FifoTransfer(0x66,packet,0) || !Command(start_tx,sizeof(start_tx),0,0)) goto failed;
    started=Board_Millis();
    do {
        if(!Interrupts(irq)) goto failed;
        if(irq[6]&0x20U) { ++g_radio.fifo_errors; Fail(RF_FIFO_ERROR); goto failed; }
        if(irq[2]&0x20U) { ++g_radio.tx_count; g_radio.stage=5; return 1U; }
        Board_DelayMs(1);
    } while((uint32_t)(Board_Millis()-started)<1000UL);
    Fail(RF_TX_TIMEOUT);
failed:
    Radio_Shutdown();
    return 0U;
}
/**
 * @brief 把待发送帧放入 TX 环形队列。
 * @param packet 待发送的固定长度帧。
 * @return 入队成功时返回 1，否则返回 0。
 */
uint8_t Radio_QueueSend(const uint8_t packet[LINK_PACKET_SIZE])
{
    uint8_t ok;
    if(g_radio.error!=RF_OK) return 0U;
    ok=PacketRing_Push(&radio_queues.tx, packet, LINK_PACKET_SIZE);
    Radio_GetTxBufferInfo(0);
    return ok;
}
/**
 * @brief 发送队首一帧并出队。
 * @return 完成时返回 1，队列为空时返回 0，故障时返回 -1。
 */
int Radio_ServiceTransmit(void)
{
    uint8_t ok;
    const uint8_t *packet;

    packet = PacketRing_Peek(&radio_queues.tx);
    if(g_radio.error!=RF_OK) return -1;
    if(!packet) return 0;
    ok=SendPacket(packet);
    PacketRing_Pop(&radio_queues.tx);
    Radio_GetTxBufferInfo(0);
    return ok ? 1 : -1;
}
/**
 * @brief 入队并同步发送一帧。
 * @param packet 待发送的固定长度帧。
 * @return 发送成功时返回 1，否则返回 0。
 */
uint8_t Radio_Send(const uint8_t packet[LINK_PACKET_SIZE])
{
    if(!Radio_QueueSend(packet)) return Fail(RF_FIFO_ERROR);
    while (radio_queues.tx.count)
        if (Radio_ServiceTransmit() < 0)
            return 0U;
    return 1U;
}
/**
 * @brief 清理 RX FIFO 和状态后启动下一次接收。
 * @return 启动成功时返回 1，否则返回 0。
 */
uint8_t Radio_StartReceive(void)
{
    /* Fixed 16-byte packet length is part of the application protocol. */
    const uint8_t start_rx[]={0x32,LINK_CHANNEL,0,0,LINK_PACKET_SIZE,8,3,3};
    uint8_t irq[8];
    uint8_t fifo[2];
    if(g_radio.error!=RF_OK) return 0U;
    if(!Ready() || !FifoInfo(2,fifo) || !Interrupts(irq) ||
       !Command(start_rx,sizeof(start_rx),0,0)) return 0U;
    g_radio.stage=7;
    return 1U;
}
/**
 * @brief 检查硬件 CRC 和长度并接收一帧。
 * @return 收到一帧时返回 1，无帧时返回 0，故障时返回 -1。
 */
int Radio_ServiceReceive(void)
{
    uint8_t irq[8];
    uint8_t fifo[2];
    uint8_t *packet;
    if(g_radio.error!=RF_OK) return -1;
    if(!Interrupts(irq)) return -1;
    if(irq[6]&0x20U) {
        ++g_radio.fifo_errors;
        return Radio_StartReceive() ? 0 : -1;
    }
    /* CRC failure has priority over PACKET_RX if both flags are latched. */
    if(irq[2]&0x08U) {
        ++g_radio.crc_errors;
        return Radio_StartReceive() ? 0 : -1;
    }
    if(!(irq[2]&0x10U)) return 0;
    if(!FifoInfo(0,fifo)) return -1;
    g_radio.last_fifo_count=fifo[0];
    if(fifo[0]!=LINK_PACKET_SIZE) {
        ++g_radio.fifo_errors;
        return Radio_StartReceive() ? 0 : -1;
    }
    packet=PacketRing_Reserve(&radio_queues.rx);
    if(!packet) {
        Radio_GetRxBufferInfo(0);
        return Radio_StartReceive() ? 0 : -1;
    }
    if(!FifoTransfer(0x77,0,packet)) return -1;
    if(!Radio_StartReceive()) return -1; /* re-arm before indicating success */
    (void)PacketRing_Commit(&radio_queues.rx, LINK_PACKET_SIZE);
    Radio_GetRxBufferInfo(0);
    ++g_radio.rx_count;
    return 1;
}
/**
 * @brief 从 RX 队列取出一帧。
 * @param packet 输出的固定长度帧缓冲区。
 * @return 取到完整帧时返回 1，否则返回 0。
 */
int Radio_ReadPacket(uint8_t packet[LINK_PACKET_SIZE])
{
    uint16_t size;

    size = PacketRing_Read(&radio_queues.rx, packet);
    Radio_GetRxBufferInfo(0);
    return size==LINK_PACKET_SIZE ? 1 : 0;
}
/**
 * @brief 轮询无线接收并读取队首帧。
 * @param packet 输出的固定长度帧缓冲区。
 * @return 无帧时返回 0，取得一帧时返回 1，故障时返回 -1。
 */
int Radio_PollReceive(uint8_t packet[LINK_PACKET_SIZE])
{
    if(Radio_ServiceReceive()<0) return -1;
    return Radio_ReadPacket(packet);
}
