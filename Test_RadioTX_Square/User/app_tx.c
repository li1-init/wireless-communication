/* 文件说明：方板应用：处理按键、对码、发送数据和确认回包。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This module is the square transmitter application state machine. */
#include "app_tx.h"
#include "board.h"
#include "radio_link.h"
#include "session_store.h"
#include "key_input.h"
#include "pairing_tx.h"
#include "indicators.h"
volatile LinkAppStatus g_link;
static uint8_t may_retry,pending_keys;
static uint32_t last_retry,last_send;
/** @brief 启动无线芯片并记录重试时间；圆板还会进入接收状态。 */
static void InitRadio(void)
{
    g_link.online=Radio_Init();
    last_retry=Board_Millis();
}
/** @brief 停止当前无线事务并记录失败，交由主循环按条件重试。 */
static void Fault(void)
{
    Radio_Shutdown();
    ++g_link.tx_failed;
    g_link.online=0U;
    g_link.pair_mode=0U;
    PairingTx_Abort();
    Indicators_Stop();
    last_retry=Board_Millis();
}
/** @brief 复制最近发送的数据帧到调试状态，方便 Watch 查看。 */
static void Remember(const uint8_t *packet)
{
    uint8_t i;
    for(i=0;i<LINK_PACKET_SIZE;++i) g_link.last_packet[i]=packet[i];
}
/** @brief 结束超时的对码尝试、熄灭提示灯并更新统计。 */
static void PairTimeout(void)
{
    g_link.pair_mode=0U;
    ++g_link.pair_timeout_count;
    Indicators_Stop();
    last_send=Board_Millis()-LINK_INTERVAL_MS;
}
/** @brief 推进方板对码事务，处理请求重发、确认匹配和截止时间。 */
static void PairPoll(void)
{
    uint8_t packet[LINK_PACKET_SIZE];
    LinkMessage message;
    int status;
    if(PairingTx_Expire(Board_Millis())) { PairTimeout(); return; }
    if(PairingTx_Request(Board_Millis(),&message)) {
        Link_Encode(packet,&message);
        Remember(packet);
        g_link.last_command=LINK_PAIR_REQUEST;
        if(!Radio_Send(packet) || !Radio_StartReceive()) { Fault(); return; }
    }
    if(PairingTx_Expire(Board_Millis())) { PairTimeout(); return; }
    status=Radio_PollReceive(packet);
    if(status<0) { Fault(); return; }
    if(status>0 && Link_Decode(packet,&message) &&
       PairingTx_AcceptAck(Board_Millis(),&message)) {
        g_link.pair_mode=0U;
        ++g_link.pair_success_count;
        Indicators_Stop();
        last_send=Board_Millis()-LINK_INTERVAL_MS;
    } else if(PairingTx_Expire(Board_Millis())) PairTimeout();
}
/** @brief 发送数据后切换接收，在限定时间内等待 ID、启动代号、token 匹配的确认。 */
static uint8_t SendDataAndWaitAck(const uint8_t *packet,const LinkMessage *sent)
{
    uint32_t started=Board_Millis();
    uint8_t reply_packet[LINK_PACKET_SIZE];
    LinkMessage reply;
    int status;
    if(!Radio_Send(packet) || !Radio_StartReceive()) return 0U;
    do {
        status=Radio_PollReceive(reply_packet);
        if(status<0) return 0U;
        if(status>0 && Link_Decode(reply_packet,&reply) &&
           reply.type==LINK_DATA_ACK && reply.payload==LINK_PAIR_ACK_OK &&
           reply.sender_id==sent->sender_id && reply.session==sent->session &&
           reply.token==sent->token) return 1U;
    } while((uint32_t)(Board_Millis()-started)<180UL);
    return 0U;
}
/** @brief 初始化方板、启动代号、按键、灯和无线模块。 */
void AppTx_Init(void)
{
    uint32_t session=0U;
    uint8_t clock_ok=Board_Init();
    g_link.role=1U;
    g_link.local_id=Board_DeviceId();
    Keys_Init();
    PairingTx_Init();
    may_retry=(uint8_t)(clock_ok && SessionStore_Next(&session));
    g_link.session=session;
    Indicators_Startup();
    if(!clock_ok) g_radio.error=RF_CLOCK_ERROR;
    else if(!may_retry) g_radio.error=RF_STORAGE_ERROR;
    else InitRadio();
    if(g_link.online) Indicators_Ready();
    last_retry=Board_Millis();
    last_send=last_retry-LINK_INTERVAL_MS;
}
/** @brief 执行一轮方板业务：按键、对码、普通发送以及确认结果处理。 */
void AppTx_Poll(void)
{
    uint32_t now=Board_Millis(),session;
    uint8_t edges,command=LINK_BEACON,packet[LINK_PACKET_SIZE],i;
    LinkMessage message;
    Keys_Update(now,Board_Keys());
    edges=Keys_Pressed();
    if(!g_link.online) {
        Indicators_Fault(now,g_radio.error);
        if(may_retry && (uint32_t)(now-last_retry)>=5000UL) InitRadio();
        return;
    }
    if((edges&KEY_CODE_MASK) && !PairingTx_Active()) {
        /* A fresh persistent epoch distinguishes attempts even after token wrap. */
        if(!SessionStore_Next(&session)) {
            g_radio.error=RF_STORAGE_ERROR;
            may_retry=0U;
            Fault();
            return;
        }
        g_link.session=session;
        now=Board_Millis();
        PairingTx_Begin(now,g_link.local_id,session,(uint8_t)(++g_link.sequence));
        g_link.pair_mode=1U;
        pending_keys=0U;
        Indicators_PairStart(now);
    }
    if(PairingTx_Active()) { PairPoll(); return; }
    pending_keys|=(uint8_t)(edges&(KEY_SW_MASK|KEY_USER_MASK));
    if((uint32_t)(now-last_send)<LINK_INTERVAL_MS &&
       (!pending_keys || (uint32_t)(now-last_send)<300UL)) return;
    for(i=0;i<2U;++i) if(pending_keys&(1U<<i)) {
        command=(uint8_t)(i+1U);
        pending_keys&=(uint8_t)~(1U<<i);
        break;
    }
    message.type=LINK_DATA;
    message.session=g_link.session;
    message.sender_id=g_link.local_id;
    message.payload=(uint8_t)((command<<3U)|(Board_Keys()&7U));
    message.token=(uint8_t)(++g_link.sequence);
    Link_Encode(packet,&message);
    Remember(packet);
    g_link.last_command=command;
    g_link.last_keys=(uint8_t)(message.payload&7U);
    if(SendDataAndWaitAck(packet,&message)) Indicators_TxPulse(Board_Millis());
    else Fault();
    last_send=Board_Millis();
}
