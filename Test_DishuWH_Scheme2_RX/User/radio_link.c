/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：无线收发驱动：执行 Si4438 命令，通过 SPI DMA 搬运 FIFO 包并管理收发队列。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 引入本模块所需的头文件接口。 */
#include "radio_link.h"
/* 引入本模块所需的头文件接口。 */
#include "radio_port.h"
/* 引入本模块所需的头文件接口。 */
#include "radio_config_b1_10k.h"
/* 引入本模块所需的头文件接口。 */
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
/* 定义编译期常量或接口宏。 */
#define RF_GPIO_PIN_CFG 0x13,1,1,1,1,0,0,0
/* 清除前面的宏定义以使用当前模块配置。 */
#undef RF_GLOBAL_XO_TUNE_2
/* 定义编译期常量或接口宏。 */
#define RF_GLOBAL_XO_TUNE_2 0x11,0x00,2,0,0x45,0x00
/* 清除前面的宏定义以使用当前模块配置。 */
#undef RF_FREQ_CONTROL_INTE_8
/* 定义编译期常量或接口宏。 */
#define RF_FREQ_CONTROL_INTE_8 0x11,0x40,8,0,0x38,0x0D,0xDD,0xDD,0x44,0x44,0x20,0xFE
/* 清除前面的宏定义以使用当前模块配置。 */
#undef RF_PA_MODE_4
/* 定义编译期常量或接口宏。 */
#define RF_PA_MODE_4 0x11,0x22,4,0,0x08,0x10,0x00,0x3D
/* 定义或初始化本模块使用的状态数据。 */
static const uint8_t configuration[]=RADIO_CONFIGURATION_DATA_ARRAY;
/* 定义或初始化本模块使用的状态数据。 */
volatile RadioLinkStatus g_radio;
/* 执行当前语句。 */
static PacketRing tx_ring,rx_ring;
/* 执行当前语句。 */
volatile PacketRingInfo g_radio_rx_buffer,g_radio_tx_buffer;
/** @brief 取得 RX 环形区快照，同时更新可在 Watch 中查看的状态。 */
/* 进入本模块的函数实现。 */
void Radio_GetRxBufferInfo(PacketRingInfo *info)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    PacketRingInfo snapshot;
/* 调用相关模块接口完成当前处理步骤。 */
    PacketRing_GetInfo(&rx_ring,&snapshot);
/* 更新运行状态或计算结果。 */
    g_radio_rx_buffer=snapshot;
/* 根据运行条件选择处理分支。 */
    if(info) *info=snapshot;
/* 结束当前代码块。 */
}
/** @brief 取得 TX 环形区快照，同时更新可在 Watch 中查看的状态。 */
/* 进入本模块的函数实现。 */
void Radio_GetTxBufferInfo(PacketRingInfo *info)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    PacketRingInfo snapshot;
/* 调用相关模块接口完成当前处理步骤。 */
    PacketRing_GetInfo(&tx_ring,&snapshot);
/* 更新运行状态或计算结果。 */
    g_radio_tx_buffer=snapshot;
/* 根据运行条件选择处理分支。 */
    if(info) *info=snapshot;
/* 结束当前代码块。 */
}
/** @brief 复制最近收到的完整帧，供上层查看；不改变队列读位置。 */
/* 进入本模块的函数实现。 */
uint16_t Radio_CopyLastReceived(uint8_t *packet,uint16_t capacity)
/* 开始当前代码块。 */
{
/* 返回当前函数的处理结果。 */
    return PacketRing_CopyLast(&rx_ring,packet,capacity);
/* 结束当前代码块。 */
}

/** @brief 记录首次无线错误并返回失败，保留最初故障原因。 */
/* 进入本模块的函数实现。 */
static uint8_t Fail(uint8_t error)
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(g_radio.error==RF_OK) g_radio.error=error;
/* 返回当前函数的处理结果。 */
    return 0U;
/* 结束当前代码块。 */
}
/** @brief 调用板级单字节 SPI，失败时记录 SPI 超时。 */
/* 进入本模块的函数实现。 */
static uint8_t Byte(uint8_t tx,uint8_t *rx)
/* 开始当前代码块。 */
{
/* 返回当前函数的处理结果。 */
    return Board_SpiByte(tx,rx) ? 1U : Fail(RF_SPI_TIMEOUT);
/* 结束当前代码块。 */
}
/** @brief 等待 SPI 发送结束后释放片选。 */
/* 进入本模块的函数实现。 */
static uint8_t End(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t ok=Board_SpiFinish();
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Nsel(1U);
/* 返回当前函数的处理结果。 */
    return ok ? 1U : Fail(RF_SPI_TIMEOUT);
/* 结束当前代码块。 */
}
/** @brief 轮询无线 CTS，准备好后读取指定长度回复。 */
/* 进入本模块的函数实现。 */
static uint8_t Response(uint8_t *reply,uint8_t length)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t start=Board_Millis();
/* 执行当前语句。 */
    uint8_t cts,discard,i;
/* 循环处理当前任务，直到满足退出条件。 */
    do {
/* 调用相关模块接口完成当前处理步骤。 */
        Board_Nsel(0U);
/* 进入本模块的函数实现。 */
        if(!Byte(0x44U,&discard)||!Byte(0xFFU,&cts)) {
/* 调用相关模块接口完成当前处理步骤。 */
            Board_Nsel(1U); return 0U;
/* 结束当前代码块。 */
        }
/* 执行当前语句。 */
        g_radio.last_cts=cts;
/* 进入本模块的函数实现。 */
        if(cts==0xFFU) {
/* 循环处理当前任务，直到满足退出条件。 */
            for(i=0;i<length;++i)
/* 进入本模块的函数实现。 */
                if(!Byte(0xFFU,reply+i)) { Board_Nsel(1U); return 0U; }
/* 返回当前函数的处理结果。 */
            return End();
/* 结束当前代码块。 */
        }
/* 根据运行条件选择处理分支。 */
        if(!End()) return 0U;
/* 调用相关模块接口完成当前处理步骤。 */
        Board_DelayMs(1U);
/* 执行当前语句。 */
    } while((uint32_t)(Board_Millis()-start)<500UL);
/* 返回当前函数的处理结果。 */
    return Fail(RF_CTS_TIMEOUT);
/* 结束当前代码块。 */
}
/** @brief 执行一次无线命令并读取回复，同时保留请求内容用于诊断。 */
/* 进入本模块的函数实现。 */
static uint8_t Command(const uint8_t *cmd,uint8_t length,uint8_t *reply,uint8_t reply_length)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    uint8_t i,discard;
/* 执行当前语句。 */
    g_radio.last_command=cmd[0];
/* 进入本模块的函数实现。 */
    if(cmd[0]!=0x23U) {
/* 执行当前语句。 */
        g_radio.request_length=length;
/* 循环处理当前任务，直到满足退出条件。 */
        for(i=0;i<16U;++i) g_radio.request_bytes[i]=(i<length)?cmd[i]:0U;
/* 结束当前代码块。 */
    }
/* 根据运行条件选择处理分支。 */
    if(!Response(0,0)) return 0U;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Nsel(0U);
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<length;++i)
/* 进入本模块的函数实现。 */
        if(!Byte(cmd[i],&discard)) { Board_Nsel(1U); return 0U; }
/* 根据运行条件选择处理分支。 */
    if(!End()) return 0U;
/* 返回当前函数的处理结果。 */
    return Response(reply,reply_length);
/* 结束当前代码块。 */
}
/** @brief 读取芯片命令错误状态并转换成驱动错误码。 */
/* 进入本模块的函数实现。 */
static uint8_t CheckError(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t cmd[]={0x23,0};
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t r[4];
/* 根据运行条件选择处理分支。 */
    if(!Command(cmd,sizeof(cmd),r,sizeof(r))) return 0U;
/* 执行当前语句。 */
    g_radio.command_error=r[2];
/* 执行当前语句。 */
    g_radio.command_error_id=r[3];
/* 根据运行条件选择处理分支。 */
    if(((r[0]|r[1])&0x08U)||r[2]) return Fail(RF_COMMAND_ERROR);
/* 返回当前函数的处理结果。 */
    return 1U;
/* 结束当前代码块。 */
}
/** @brief 发送配置命令后检查无线芯片是否报告错误。 */
/* 进入本模块的函数实现。 */
static uint8_t Checked(const uint8_t *cmd,uint8_t length)
/* 开始当前代码块。 */
{
/* 返回当前函数的处理结果。 */
    return Command(cmd,length,0,0) && CheckError();
/* 结束当前代码块。 */
}
/** @brief 读取并清除无线中断状态，保存收发与芯片事件。 */
/* 进入本模块的函数实现。 */
static uint8_t Interrupts(uint8_t r[8])
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t cmd[]={0x20,0,0,0};
/* 根据运行条件选择处理分支。 */
    if(!Command(cmd,sizeof(cmd),r,8)) return 0U;
/* 执行当前语句。 */
    g_radio.last_ph_pending=r[2];
/* 执行当前语句。 */
    g_radio.last_chip_pending=r[6];
/* 进入本模块的函数实现。 */
    if(r[6]&0x08U) {
/* 执行当前语句。 */
        (void)CheckError();
/* 返回当前函数的处理结果。 */
        return Fail(RF_COMMAND_ERROR);
/* 结束当前代码块。 */
    }
/* 返回当前函数的处理结果。 */
    return 1U;
/* 结束当前代码块。 */
}
/** @brief 查询 FIFO 字节数，按参数选择是否复位收发 FIFO。 */
/* 进入本模块的函数实现。 */
static uint8_t FifoInfo(uint8_t reset,uint8_t reply[2])
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t cmd[]={0x15,reset};
/* 返回当前函数的处理结果。 */
    return Command(cmd,sizeof(cmd),reply,2U);
/* 结束当前代码块。 */
}
/** @brief 发送 FIFO 操作码，再以 DMA 搬运一个完整包，最后释放片选。 */
/* 进入本模块的函数实现。 */
static uint8_t FifoTransfer(uint8_t opcode,const uint8_t *tx,uint8_t *rx)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t b;
/* 根据运行条件选择处理分支。 */
    if(!Response(0,0)) return 0U;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Nsel(0U);
/* 进入本模块的函数实现。 */
    if(!Byte(opcode,&b)) { Board_Nsel(1U); return 0U; }
/* 进入本模块的函数实现。 */
    if(!SpiDma_Transfer(tx,rx,LINK_PACKET_SIZE)) {
/* 调用相关模块接口完成当前处理步骤。 */
        Board_Nsel(1U);
/* 返回当前函数的处理结果。 */
        return Fail(RF_DMA_ERROR);
/* 结束当前代码块。 */
    }
/* 返回当前函数的处理结果。 */
    return End();
/* 结束当前代码块。 */
}
/** @brief 把无线芯片切换到 READY，并轮询确认状态。 */
/* 进入本模块的函数实现。 */
static uint8_t Ready(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t cmd[]={0x34,3};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t query=0x33;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t reply[2];
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t start;
/* 根据运行条件选择处理分支。 */
    if(!Command(cmd,sizeof(cmd),0,0)) return 0U;
/* 更新运行状态或计算结果。 */
    start=Board_Millis();
/* 循环处理当前任务，直到满足退出条件。 */
    do {
/* 根据运行条件选择处理分支。 */
        if(!Command(&query,1,reply,2)) return 0U;
/* 执行当前语句。 */
        g_radio.device_state=reply[0];
/* 根据运行条件选择处理分支。 */
        if((reply[0]&0x0FU)==3U) return 1U;
/* 调用相关模块接口完成当前处理步骤。 */
        Board_DelayMs(1);
/* 执行当前语句。 */
    } while((uint32_t)(Board_Millis()-start)<100UL);
/* 返回当前函数的处理结果。 */
    return Fail(RF_COMMAND_ERROR);
/* 结束当前代码块。 */
}
/** @brief 读回无线配置属性，与预期值逐字节比较。 */
/* 进入本模块的函数实现。 */
static uint8_t Verify(uint8_t group,uint8_t offset,const uint8_t *expected,uint8_t length)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t cmd[]={0x12,group,length,offset}, reply[12],i;
/* 根据运行条件选择处理分支。 */
    if(length>sizeof(reply)) return Fail(RF_CONFIG_ERROR);
/* 根据运行条件选择处理分支。 */
    if(!Command(cmd,sizeof(cmd),reply,length)) return 0U;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<length;++i)
/* 根据运行条件选择处理分支。 */
        if(reply[i]!=expected[i]) return Fail(RF_CONFIG_ERROR);
/* 返回当前函数的处理结果。 */
    return CheckError();
/* 结束当前代码块。 */
}
/** @brief 配置固定包长、同步字、CRC 和中断，并读回核对关键参数。 */
/* 进入本模块的函数实现。 */
static uint8_t ConfigurePacket(void)
/* 开始当前代码块。 */
{
    /* B1 packet handler uses one shared field table. Property 1207
     * (PKT_CONFIG2) and C2's separate RX fields must never be addressed. */
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t crc[]={0x11,0x12,1,0,0x85};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t packet[]={0x11,0x12,1,6,0x02};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t fixed_length[]={0x11,0x12,3,8,0,0,0};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t field[]={0x11,0x12,4,0x0D,0,LINK_PACKET_SIZE,0,0xAA};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t field2[]={0x11,0x12,4,0x11,0,0,0,0};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t field3[]={0x11,0x12,4,0x15,0,0,0,0};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t field4[]={0x11,0x12,4,0x19,0,0,0,0};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t field5[]={0x11,0x12,4,0x1D,0,0,0,0};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t interrupts[]={0x11,0x01,4,0,0x05,0x38,0x00,0x28};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t expected_freq[]={0x38,0x0D,0xDD,0xDD,0x44,0x44,0x20,0xFE};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t expected_sync[]={1,0xB4,0x2B};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t expected_mod[]={3,0,7,0x06,0x1A,0x80};
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t expected_pa[]={0x08,0x10,0,0x3D};
/* 根据运行条件选择处理分支。 */
    if(!Checked(crc,sizeof(crc)) || !Checked(packet,sizeof(packet)) ||
/* 说明本行代码在当前模块中的处理作用。 */
       !Checked(fixed_length,sizeof(fixed_length)) || !Checked(field,sizeof(field)) ||
/* 说明本行代码在当前模块中的处理作用。 */
       !Checked(field2,sizeof(field2)) || !Checked(field3,sizeof(field3)) ||
/* 说明本行代码在当前模块中的处理作用。 */
       !Checked(field4,sizeof(field4)) || !Checked(field5,sizeof(field5)) ||
/* 执行当前语句。 */
       !Checked(interrupts,sizeof(interrupts))) return 0U;
/* 执行当前语句。 */
    g_radio.stage=4;
/* 返回当前函数的处理结果。 */
    return Verify(0x12,0,crc+4,1) && Verify(0x12,6,packet+4,1) &&
/* 调用相关模块接口完成当前处理步骤。 */
           Verify(0x12,8,fixed_length+4,3) &&
/* 调用相关模块接口完成当前处理步骤。 */
           Verify(0x12,0x0D,field+4,4) && Verify(0x01,0,interrupts+4,4) &&
/* 调用相关模块接口完成当前处理步骤。 */
           Verify(0x40,0,expected_freq,8) && Verify(0x11,0,expected_sync,3) &&
/* 调用相关模块接口完成当前处理步骤。 */
           Verify(0x20,0,expected_mod,6) && Verify(0x22,0,expected_pa,4);
/* 结束当前代码块。 */
}
/** @brief 释放片选并关闭无线芯片。 */
/* 进入本模块的函数实现。 */
void Radio_Shutdown(void)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Nsel(1U);
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Sdn(1U);
/* 结束当前代码块。 */
}
/** @brief 重置包队列，初始化无线芯片并校验型号和配置。 */
/* 进入本模块的函数实现。 */
uint8_t Radio_Init(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t pos=0;
/* 执行当前语句。 */
    uint8_t n,reply[8],fifo[2],i;
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t part=0x01;
/* 调用相关模块接口完成当前处理步骤。 */
    PacketRing_Init(&tx_ring);
/* 调用相关模块接口完成当前处理步骤。 */
    PacketRing_Init(&rx_ring);
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_GetRxBufferInfo(0);
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_GetTxBufferInfo(0);
/* 更新运行状态或计算结果。 */
    ++g_radio.init_count;
/* 执行当前语句。 */
    g_radio.error=RF_OK;
/* 执行当前语句。 */
    g_radio.command_error=g_radio.command_error_id=0;
/* 执行当前语句。 */
    g_radio.config_offset=0;
/* 执行当前语句。 */
    g_radio.profile_version=2U;
/* 执行当前语句。 */
    g_radio.device_state=0U;
/* 执行当前语句。 */
    g_radio.stage=1;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_RadioInit();
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Sdn(1U); Board_DelayMs(10);
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Sdn(0U); Board_DelayMs(20);
    /* First WDS command is exactly POWER_UP (7 bytes). */
/* 进入本模块的函数实现。 */
    if(configuration[0]!=7 || configuration[1]!=0x02) {
/* 调用相关模块接口完成当前处理步骤。 */
        Fail(RF_CONFIG_ERROR); goto failed;
/* 结束当前代码块。 */
    }
/* 根据运行条件选择处理分支。 */
    if(!Checked(configuration+1,7)) goto failed;
/* 更新运行状态或计算结果。 */
    pos=8;
/* 执行当前语句。 */
    g_radio.stage=2;
/* 根据运行条件选择处理分支。 */
    if(!Command(&part,1,reply,8)) goto failed;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<8;++i) g_radio.part_info[i]=reply[i];
/* 执行当前语句。 */
    g_radio.part=(uint16_t)((uint16_t)reply[1]<<8)|reply[2];
/* 进入本模块的函数实现。 */
    if(g_radio.part!=0x4438U) { Fail(RF_WRONG_PART); goto failed; }
    /* The square TX has 0x11/0x03; the round RX currently on the bench
     * reports 0x22/0x06. Both identify the same 0x4438 family and must be
     * allowed to proceed with the common B1-compatible profile. */
/* 根据运行条件选择处理分支。 */
    if(!((reply[0]==0x11U && reply[7]==0x03U) ||
/* 说明本行代码在当前模块中的处理作用。 */
         (reply[0]==0x22U && reply[7]==0x06U))) {
/* 调用相关模块接口完成当前处理步骤。 */
        Fail(RF_UNSUPPORTED_REV); goto failed;
/* 结束当前代码块。 */
    }
/* 执行当前语句。 */
    g_radio.stage=3;
/* 进入本模块的函数实现。 */
    while(pos<sizeof(configuration)) {
/* 执行当前语句。 */
        g_radio.config_offset=pos;
/* 更新运行状态或计算结果。 */
        n=configuration[pos++];
/* 根据运行条件选择处理分支。 */
        if(n==0U) break;
/* 进入本模块的函数实现。 */
        if(n>16U || (uint32_t)pos+n>sizeof(configuration)) {
/* 调用相关模块接口完成当前处理步骤。 */
            Fail(RF_CONFIG_ERROR); goto failed;
/* 结束当前代码块。 */
        }
/* 根据运行条件选择处理分支。 */
        if(!Checked(configuration+pos,n)) goto failed;
/* 更新运行状态或计算结果。 */
        pos=(uint16_t)(pos+n);
/* 结束当前代码块。 */
    }
/* 进入本模块的函数实现。 */
    if(pos!=sizeof(configuration) || configuration[pos-1]!=0) {
/* 调用相关模块接口完成当前处理步骤。 */
        Fail(RF_CONFIG_ERROR); goto failed;
/* 结束当前代码块。 */
    }
/* 进入本模块的函数实现。 */
    if(!ConfigurePacket() || !Ready() || !FifoInfo(3,fifo) || !Interrupts(reply))
/* 定义或初始化本模块使用的状态数据。 */
        goto failed;
/* 执行当前语句。 */
    g_radio.stage=5;
/* 返回当前函数的处理结果。 */
    return 1U;
/* 说明本行代码在当前模块中的处理作用。 */
failed:
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_Shutdown();
/* 返回当前函数的处理结果。 */
    return 0U;
/* 结束当前代码块。 */
}
/** @brief 从队列槽位 DMA 写入无线 FIFO，等待芯片报告 PACKET_SENT。 */
/* 进入本模块的函数实现。 */
static uint8_t SendPacket(const uint8_t packet[LINK_PACKET_SIZE])
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t start_tx[]={0x31,LINK_CHANNEL,0x30,0,0,0,0};
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t irq[8],fifo[2];
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t started;
/* 根据运行条件选择处理分支。 */
    if(g_radio.error!=RF_OK) return 0U;
/* 执行当前语句。 */
    g_radio.stage=6;
/* 根据运行条件选择处理分支。 */
    if(!Ready() || !Interrupts(irq) || !FifoInfo(1,fifo)) goto failed;
/* 进入本模块的函数实现。 */
    if(fifo[1]<LINK_PACKET_SIZE) { Fail(RF_FIFO_ERROR); goto failed; }
/* 根据运行条件选择处理分支。 */
    if(!FifoTransfer(0x66,packet,0) || !Command(start_tx,sizeof(start_tx),0,0)) goto failed;
/* 更新运行状态或计算结果。 */
    started=Board_Millis();
/* 循环处理当前任务，直到满足退出条件。 */
    do {
/* 根据运行条件选择处理分支。 */
        if(!Interrupts(irq)) goto failed;
/* 进入本模块的函数实现。 */
        if(irq[6]&0x20U) { ++g_radio.fifo_errors; Fail(RF_FIFO_ERROR); goto failed; }
/* 进入本模块的函数实现。 */
        if(irq[2]&0x20U) { ++g_radio.tx_count; g_radio.stage=5; return 1U; }
/* 调用相关模块接口完成当前处理步骤。 */
        Board_DelayMs(1);
/* 执行当前语句。 */
    } while((uint32_t)(Board_Millis()-started)<1000UL);
/* 调用相关模块接口完成当前处理步骤。 */
    Fail(RF_TX_TIMEOUT);
/* 说明本行代码在当前模块中的处理作用。 */
failed:
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_Shutdown();
/* 返回当前函数的处理结果。 */
    return 0U;
/* 结束当前代码块。 */
}
/** @brief 把待发送帧放入 TX 环形队列，满时返回失败。 */
/* 进入本模块的函数实现。 */
uint8_t Radio_QueueSend(const uint8_t packet[LINK_PACKET_SIZE])
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t ok;
/* 根据运行条件选择处理分支。 */
    if(g_radio.error!=RF_OK) return 0U;
/* 更新运行状态或计算结果。 */
    ok=PacketRing_Push(&tx_ring,packet,LINK_PACKET_SIZE);
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_GetTxBufferInfo(0);
/* 返回当前函数的处理结果。 */
    return ok;
/* 结束当前代码块。 */
}
/** @brief 发送队首一包并出队；1 为完成，0 为空，-1 为故障。 */
/* 进入本模块的函数实现。 */
int Radio_ServiceTransmit(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t ok;
/* 执行当前语句。 */
    const uint8_t *packet=PacketRing_Peek(&tx_ring);
/* 根据运行条件选择处理分支。 */
    if(g_radio.error!=RF_OK) return -1;
/* 根据运行条件选择处理分支。 */
    if(!packet) return 0;
/* 更新运行状态或计算结果。 */
    ok=SendPacket(packet);
/* 调用相关模块接口完成当前处理步骤。 */
    PacketRing_Pop(&tx_ring);
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_GetTxBufferInfo(0);
/* 返回当前函数的处理结果。 */
    return ok ? 1 : -1;
/* 结束当前代码块。 */
}
/** @brief 兼容同步发送接口：入队并发送排队包；本地发送完成不等于对端收到。 */
/* 进入本模块的函数实现。 */
uint8_t Radio_Send(const uint8_t packet[LINK_PACKET_SIZE])
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(!Radio_QueueSend(packet)) return Fail(RF_FIFO_ERROR);
/* 循环处理当前任务，直到满足退出条件。 */
    while(tx_ring.count) if(Radio_ServiceTransmit()<0) return 0U;
/* 返回当前函数的处理结果。 */
    return 1U;
/* 结束当前代码块。 */
}
/** @brief 清理无线 RX FIFO 和状态后启动下一包接收。 */
/* 进入本模块的函数实现。 */
uint8_t Radio_StartReceive(void)
/* 开始当前代码块。 */
{
    /* Fixed 16-byte packet length is part of the application protocol. */
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t start_rx[]={0x32,LINK_CHANNEL,0,0,LINK_PACKET_SIZE,8,3,3};
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t irq[8],fifo[2];
/* 根据运行条件选择处理分支。 */
    if(g_radio.error!=RF_OK) return 0U;
/* 根据运行条件选择处理分支。 */
    if(!Ready() || !FifoInfo(2,fifo) || !Interrupts(irq) ||
/* 执行当前语句。 */
       !Command(start_rx,sizeof(start_rx),0,0)) return 0U;
/* 执行当前语句。 */
    g_radio.stage=7;
/* 返回当前函数的处理结果。 */
    return 1U;
/* 结束当前代码块。 */
}
/** @brief 检查硬件 CRC 与长度，DMA 读 FIFO 到 RX 空槽，完成后入队。 */
/* 进入本模块的函数实现。 */
int Radio_ServiceReceive(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t irq[8],fifo[2];
/* 执行当前语句。 */
    uint8_t *packet;
/* 根据运行条件选择处理分支。 */
    if(g_radio.error!=RF_OK) return -1;
/* 根据运行条件选择处理分支。 */
    if(!Interrupts(irq)) return -1;
/* 进入本模块的函数实现。 */
    if(irq[6]&0x20U) {
/* 更新运行状态或计算结果。 */
        ++g_radio.fifo_errors;
/* 返回当前函数的处理结果。 */
        return Radio_StartReceive() ? 0 : -1;
/* 结束当前代码块。 */
    }
    /* CRC failure has priority over PACKET_RX if both flags are latched. */
/* 进入本模块的函数实现。 */
    if(irq[2]&0x08U) {
/* 更新运行状态或计算结果。 */
        ++g_radio.crc_errors;
/* 返回当前函数的处理结果。 */
        return Radio_StartReceive() ? 0 : -1;
/* 结束当前代码块。 */
    }
/* 根据运行条件选择处理分支。 */
    if(!(irq[2]&0x10U)) return 0;
/* 根据运行条件选择处理分支。 */
    if(!FifoInfo(0,fifo)) return -1;
/* 执行当前语句。 */
    g_radio.last_fifo_count=fifo[0];
/* 进入本模块的函数实现。 */
    if(fifo[0]!=LINK_PACKET_SIZE) {
/* 更新运行状态或计算结果。 */
        ++g_radio.fifo_errors;
/* 返回当前函数的处理结果。 */
        return Radio_StartReceive() ? 0 : -1;
/* 结束当前代码块。 */
    }
/* 更新运行状态或计算结果。 */
    packet=PacketRing_Reserve(&rx_ring);
/* 进入本模块的函数实现。 */
    if(!packet) {
/* 调用相关模块接口完成当前处理步骤。 */
        Radio_GetRxBufferInfo(0);
/* 返回当前函数的处理结果。 */
        return Radio_StartReceive() ? 0 : -1;
/* 结束当前代码块。 */
    }
/* 根据运行条件选择处理分支。 */
    if(!FifoTransfer(0x77,0,packet)) return -1;
/* 根据运行条件选择处理分支。 */
    if(!Radio_StartReceive()) return -1; /* re-arm before indicating success */
/* 执行当前语句。 */
    (void)PacketRing_Commit(&rx_ring,LINK_PACKET_SIZE);
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_GetRxBufferInfo(0);
/* 更新运行状态或计算结果。 */
    ++g_radio.rx_count;
/* 返回当前函数的处理结果。 */
    return 1;
/* 结束当前代码块。 */
}
/** @brief 从 RX 队列取一包给应用；返回 1 表示取到了完整帧。 */
/* 进入本模块的函数实现。 */
int Radio_ReadPacket(uint8_t packet[LINK_PACKET_SIZE])
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t size=PacketRing_Read(&rx_ring,packet);
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_GetRxBufferInfo(0);
/* 返回当前函数的处理结果。 */
    return size==LINK_PACKET_SIZE ? 1 : 0;
/* 结束当前代码块。 */
}
/** @brief 先轮询无线接收，再取队首包，保留原上层调用方式。 */
/* 进入本模块的函数实现。 */
int Radio_PollReceive(uint8_t packet[LINK_PACKET_SIZE])
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(Radio_ServiceReceive()<0) return -1;
/* 返回当前函数的处理结果。 */
    return Radio_ReadPacket(packet);
/* 结束当前代码块。 */
}
