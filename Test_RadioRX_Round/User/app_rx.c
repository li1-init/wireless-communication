/* 文件说明：圆板应用：处理 SW 长按、对码保存、数据接收和确认回包。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This module is the round receiver application and acknowledgement loop. */
#include "app_rx.h"
#include "board.h"
#include "radio_link.h"
#include "key_input.h"
#include "pair_store.h"
#include "pairing_rx.h"
volatile LinkAppStatus g_link;
static uint8_t clock_ok;
static uint32_t last_retry;
/** @brief 启动无线芯片并记录重试时间；圆板还会进入接收状态。 */
static void InitRadio(void)
{
    g_link.online=(uint8_t)(Radio_Init() && Radio_StartReceive());
    last_retry=Board_Millis();
    if(!g_link.online) Board_Sound(SOUND_FAULT,g_radio.error);
}
/** @brief 停止当前无线事务并记录失败，交由主循环按条件重试。 */
static void Fault(void)
{
    Radio_Shutdown();
    PairingRx_CancelReply();
    g_link.online=0U;
    last_retry=Board_Millis();
    Board_Sound(SOUND_FAULT,g_radio.error);
}
/** @brief 初始化圆板、读取配对名单并启动无线接收。 */
void AppRx_Init(void)
{
    clock_ok=Board_Init();
    g_link.role=2U;
    g_link.local_id=Board_DeviceId();
    Keys_Init();
    PairingRx_Init();
    if(clock_ok) PairStore_Init();
    Board_Sound(SOUND_BOOT,0U);
    Board_DelayMs(400U);
    if(clock_ok) InitRadio();
    else {
        g_radio.error=RF_CLOCK_ERROR;
        Board_Sound(SOUND_FAULT,RF_CLOCK_ERROR);
    }
}
/** @brief 执行一轮圆板业务：长按切换、解包、反馈和确认回包。 */
void AppRx_Poll(void)
{
    uint32_t now=Board_Millis();
    uint8_t packet[LINK_PACKET_SIZE],i;
    LinkMessage message;
    int status;
    Keys_Update(now,Board_Keys());
    if(Keys_LongPress(KEY_SW_MASK,now,2000UL)) {
        g_link.pair_mode=PairingRx_Toggle(now);
        Board_Sound(g_link.pair_mode?SOUND_PAIR_ENTER:SOUND_PAIR_EXIT,0U);
    }
    if(!g_link.online) {
        if(clock_ok && (uint32_t)(now-last_retry)>=5000UL) InitRadio();
        return;
    }
    status=Radio_PollReceive(packet);
    if(status<0) { Fault(); return; }
    if(status>0) {
        for(i=0;i<LINK_PACKET_SIZE;++i) g_link.last_packet[i]=packet[i];
        if(!Link_Decode(packet,&message)) ++g_link.rx_rejected_count;
        else if(message.type==LINK_PAIR_REQUEST) {
            if(!PairingRx_OnRequest(Board_Millis(),&message)) ++g_link.rx_rejected_count;
        } else if(!PairingRx_Active() && PairStore_AcceptData(&message)) {
            g_link.sender_locked=1U;
            g_link.last_sender_id=message.sender_id;
            g_link.last_sequence=message.token;
            g_link.session=message.session;
            ++g_link.rx_valid_count;
            g_link.last_command=(uint8_t)(message.payload>>3);
            g_link.last_keys=(uint8_t)(message.payload&7U);
            g_link.last_valid_ms=Board_Millis();
            Board_Sound(SOUND_PACKET,0U);
            {
                LinkMessage ack=message;
                ack.type=LINK_DATA_ACK;
                ack.payload=LINK_PAIR_ACK_OK;
                Link_Encode(packet,&ack);
                (void)Radio_Send(packet);
                (void)Radio_StartReceive();
            }
        } else ++g_link.rx_rejected_count;
    }
    if(PairingRx_GetReply(Board_Millis(),&message)) {
        Link_Encode(packet,&message);
        if(!Radio_Send(packet) || !Radio_StartReceive()) { Fault(); return; }
        ++g_link.pair_ack_count;
        if(PairingRx_ReplySent()) {
            ++g_link.pair_success_count;
            Board_Sound(SOUND_PAIR_OK,0U);
        }
    }
}
