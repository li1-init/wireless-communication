/* 文件说明：无线收发驱动：执行 Si4438 命令，通过 SPI DMA 搬运 FIFO 包并管理收发队列。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
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
#undef RF_GPIO_PIN_CFG
#define RF_GPIO_PIN_CFG 0x13,1,1,1,1,0,0,0
#undef RF_GLOBAL_XO_TUNE_2
#define RF_GLOBAL_XO_TUNE_2 0x11,0x00,2,0,0x45,0x00
#undef RF_FREQ_CONTROL_INTE_8
#define RF_FREQ_CONTROL_INTE_8 0x11,0x40,8,0,0x38,0x0D,0xDD,0xDD,0x44,0x44,0x20,0xFE
#undef RF_PA_MODE_4
#define RF_PA_MODE_4 0x11,0x22,4,0,0x08,0x10,0x00,0x3D
static const uint8_t configuration[]=RADIO_CONFIGURATION_DATA_ARRAY;
volatile RadioLinkStatus g_radio;
static PacketRing tx_ring,rx_ring;
volatile PacketRingInfo g_radio_rx_buffer,g_radio_tx_buffer;
/** @brief 取得 RX 环形区快照，同时更新可在 Watch 中查看的状态。 */
void Radio_GetRxBufferInfo(PacketRingInfo *info)
{
    PacketRingInfo snapshot;
    PacketRing_GetInfo(&rx_ring,&snapshot);
    g_radio_rx_buffer=snapshot;
    if(info) *info=snapshot;
}
/** @brief 取得 TX 环形区快照，同时更新可在 Watch 中查看的状态。 */
void Radio_GetTxBufferInfo(PacketRingInfo *info)
{
    PacketRingInfo snapshot;
    PacketRing_GetInfo(&tx_ring,&snapshot);
    g_radio_tx_buffer=snapshot;
    if(info) *info=snapshot;
}
/** @brief 复制最近收到的完整帧，供上层查看；不改变队列读位置。 */
uint16_t Radio_CopyLastReceived(uint8_t *packet,uint16_t capacity)
{
    return PacketRing_CopyLast(&rx_ring,packet,capacity);
}

/** @brief 记录首次无线错误并返回失败，保留最初故障原因。 */
static uint8_t Fail(uint8_t error)
{
    if(g_radio.error==RF_OK) g_radio.error=error;
    return 0U;
}
/** @brief 调用板级单字节 SPI，失败时记录 SPI 超时。 */
static uint8_t Byte(uint8_t tx,uint8_t *rx)
{
    return Board_SpiByte(tx,rx) ? 1U : Fail(RF_SPI_TIMEOUT);
}
/** @brief 等待 SPI 发送结束后释放片选。 */
static uint8_t End(void)
{
    uint8_t ok=Board_SpiFinish();
    Board_Nsel(1U);
    return ok ? 1U : Fail(RF_SPI_TIMEOUT);
}
/** @brief 轮询无线 CTS，准备好后读取指定长度回复。 */
static uint8_t Response(uint8_t *reply,uint8_t length)
{
    uint32_t start=Board_Millis();
    uint8_t cts,discard,i;
    do {
        Board_Nsel(0U);
        if(!Byte(0x44U,&discard)||!Byte(0xFFU,&cts)) {
            Board_Nsel(1U); return 0U;
        }
        g_radio.last_cts=cts;
        if(cts==0xFFU) {
            for(i=0;i<length;++i)
                if(!Byte(0xFFU,reply+i)) { Board_Nsel(1U); return 0U; }
            return End();
        }
        if(!End()) return 0U;
        Board_DelayMs(1U);
    } while((uint32_t)(Board_Millis()-start)<500UL);
    return Fail(RF_CTS_TIMEOUT);
}
/** @brief 执行一次无线命令并读取回复，同时保留请求内容用于诊断。 */
static uint8_t Command(const uint8_t *cmd,uint8_t length,uint8_t *reply,uint8_t reply_length)
{
    uint8_t i,discard;
    g_radio.last_command=cmd[0];
    if(cmd[0]!=0x23U) {
        g_radio.request_length=length;
        for(i=0;i<16U;++i) g_radio.request_bytes[i]=(i<length)?cmd[i]:0U;
    }
    if(!Response(0,0)) return 0U;
    Board_Nsel(0U);
    for(i=0;i<length;++i)
        if(!Byte(cmd[i],&discard)) { Board_Nsel(1U); return 0U; }
    if(!End()) return 0U;
    return Response(reply,reply_length);
}
/** @brief 读取芯片命令错误状态并转换成驱动错误码。 */
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
/** @brief 发送配置命令后检查无线芯片是否报告错误。 */
static uint8_t Checked(const uint8_t *cmd,uint8_t length)
{
    return Command(cmd,length,0,0) && CheckError();
}
/** @brief 读取并清除无线中断状态，保存收发与芯片事件。 */
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
/** @brief 查询 FIFO 字节数，按参数选择是否复位收发 FIFO。 */
static uint8_t FifoInfo(uint8_t reset,uint8_t reply[2])
{
    const uint8_t cmd[]={0x15,reset};
    return Command(cmd,sizeof(cmd),reply,2U);
}
/** @brief 发送 FIFO 操作码，再以 DMA 搬运一个完整包，最后释放片选。 */
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
/** @brief 把无线芯片切换到 READY，并轮询确认状态。 */
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
/** @brief 读回无线配置属性，与预期值逐字节比较。 */
static uint8_t Verify(uint8_t group,uint8_t offset,const uint8_t *expected,uint8_t length)
{
    uint8_t cmd[]={0x12,group,length,offset}, reply[12],i;
    if(length>sizeof(reply)) return Fail(RF_CONFIG_ERROR);
    if(!Command(cmd,sizeof(cmd),reply,length)) return 0U;
    for(i=0;i<length;++i)
        if(reply[i]!=expected[i]) return Fail(RF_CONFIG_ERROR);
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
/** @brief 释放片选并关闭无线芯片。 */
void Radio_Shutdown(void)
{
    Board_Nsel(1U);
    Board_Sdn(1U);
}
/** @brief 重置包队列，初始化无线芯片并校验型号和配置。 */
uint8_t Radio_Init(void)
{
    uint16_t pos=0;
    uint8_t n,reply[8],fifo[2],i;
    const uint8_t part=0x01;
    PacketRing_Init(&tx_ring);
    PacketRing_Init(&rx_ring);
    Radio_GetRxBufferInfo(0);
    Radio_GetTxBufferInfo(0);
    ++g_radio.init_count;
    g_radio.error=RF_OK;
    g_radio.command_error=g_radio.command_error_id=0;
    g_radio.config_offset=0;
    g_radio.profile_version=2U;
    g_radio.device_state=0U;
    g_radio.stage=1;
    Board_RadioInit();
    Board_Sdn(1U); Board_DelayMs(10);
    Board_Sdn(0U); Board_DelayMs(20);
    /* First WDS command is exactly POWER_UP (7 bytes). */
    if(configuration[0]!=7 || configuration[1]!=0x02) {
        Fail(RF_CONFIG_ERROR); goto failed;
    }
    if(!Checked(configuration+1,7)) goto failed;
    pos=8;
    g_radio.stage=2;
    if(!Command(&part,1,reply,8)) goto failed;
    for(i=0;i<8;++i) g_radio.part_info[i]=reply[i];
    g_radio.part=(uint16_t)((uint16_t)reply[1]<<8)|reply[2];
    if(g_radio.part!=0x4438U) { Fail(RF_WRONG_PART); goto failed; }
    if(reply[0]!=0x11U || reply[7]!=0x03U) {
        Fail(RF_UNSUPPORTED_REV); goto failed;
    }
    g_radio.stage=3;
    while(pos<sizeof(configuration)) {
        g_radio.config_offset=pos;
        n=configuration[pos++];
        if(n==0U) break;
        if(n>16U || (uint32_t)pos+n>sizeof(configuration)) {
            Fail(RF_CONFIG_ERROR); goto failed;
        }
        if(!Checked(configuration+pos,n)) goto failed;
        pos=(uint16_t)(pos+n);
    }
    if(pos!=sizeof(configuration) || configuration[pos-1]!=0) {
        Fail(RF_CONFIG_ERROR); goto failed;
    }
    if(!ConfigurePacket() || !Ready() || !FifoInfo(3,fifo) || !Interrupts(reply))
        goto failed;
    g_radio.stage=5;
    return 1U;
failed:
    Radio_Shutdown();
    return 0U;
}
/** @brief 从队列槽位 DMA 写入无线 FIFO，等待芯片报告 PACKET_SENT。 */
static uint8_t SendPacket(const uint8_t packet[LINK_PACKET_SIZE])
{
    const uint8_t start_tx[]={0x31,LINK_CHANNEL,0x30,0,0,0,0};
    uint8_t irq[8],fifo[2];
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
/** @brief 把待发送帧放入 TX 环形队列，满时返回失败。 */
uint8_t Radio_QueueSend(const uint8_t packet[LINK_PACKET_SIZE])
{
    uint8_t ok;
    if(g_radio.error!=RF_OK) return 0U;
    ok=PacketRing_Push(&tx_ring,packet,LINK_PACKET_SIZE);
    Radio_GetTxBufferInfo(0);
    return ok;
}
/** @brief 发送队首一包并出队；1 为完成，0 为空，-1 为故障。 */
int Radio_ServiceTransmit(void)
{
    uint8_t ok;
    const uint8_t *packet=PacketRing_Peek(&tx_ring);
    if(g_radio.error!=RF_OK) return -1;
    if(!packet) return 0;
    ok=SendPacket(packet);
    PacketRing_Pop(&tx_ring);
    Radio_GetTxBufferInfo(0);
    return ok ? 1 : -1;
}
/** @brief 兼容同步发送接口：入队并发送排队包；本地发送完成不等于对端收到。 */
uint8_t Radio_Send(const uint8_t packet[LINK_PACKET_SIZE])
{
    if(!Radio_QueueSend(packet)) return Fail(RF_FIFO_ERROR);
    while(tx_ring.count) if(Radio_ServiceTransmit()<0) return 0U;
    return 1U;
}
/** @brief 清理无线 RX FIFO 和状态后启动下一包接收。 */
uint8_t Radio_StartReceive(void)
{
    /* Fixed 16-byte packet length is part of the application protocol. */
    const uint8_t start_rx[]={0x32,LINK_CHANNEL,0,0,LINK_PACKET_SIZE,8,3,3};
    uint8_t irq[8],fifo[2];
    if(g_radio.error!=RF_OK) return 0U;
    if(!Ready() || !FifoInfo(2,fifo) || !Interrupts(irq) ||
       !Command(start_rx,sizeof(start_rx),0,0)) return 0U;
    g_radio.stage=7;
    return 1U;
}
/** @brief 检查硬件 CRC 与长度，DMA 读 FIFO 到 RX 空槽，完成后入队。 */
int Radio_ServiceReceive(void)
{
    uint8_t irq[8],fifo[2];
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
    packet=PacketRing_Reserve(&rx_ring);
    if(!packet) {
        Radio_GetRxBufferInfo(0);
        return Radio_StartReceive() ? 0 : -1;
    }
    if(!FifoTransfer(0x77,0,packet)) return -1;
    if(!Radio_StartReceive()) return -1; /* re-arm before indicating success */
    (void)PacketRing_Commit(&rx_ring,LINK_PACKET_SIZE);
    Radio_GetRxBufferInfo(0);
    ++g_radio.rx_count;
    return 1;
}
/** @brief 从 RX 队列取一包给应用；返回 1 表示取到了完整帧。 */
int Radio_ReadPacket(uint8_t packet[LINK_PACKET_SIZE])
{
    uint16_t size=PacketRing_Read(&rx_ring,packet);
    Radio_GetRxBufferInfo(0);
    return size==LINK_PACKET_SIZE ? 1 : 0;
}
/** @brief 先轮询无线接收，再取队首包，保留原上层调用方式。 */
int Radio_PollReceive(uint8_t packet[LINK_PACKET_SIZE])
{
    if(Radio_ServiceReceive()<0) return -1;
    return Radio_ReadPacket(packet);
}
