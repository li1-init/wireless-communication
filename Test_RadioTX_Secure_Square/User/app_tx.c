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
#include "tx_peer.h"
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
    int status;
    if(PairingTx_Expire(Board_Millis())) { PairTimeout(); return; }
    if(PairingTx_Request(Board_Millis(),packet)) {
        Remember(packet);
        g_link.last_command=LINK_PAIR_REQUEST;
        if(!Radio_Send(packet) || !Radio_StartReceive()) { Fault(); return; }
    }
    if(PairingTx_Expire(Board_Millis())) { PairTimeout(); return; }
    status=Radio_PollReceive(packet);
    if(status<0) { Fault(); return; }
    if(status>0 && PairingTx_AcceptAck(Board_Millis(),packet)) {
        g_link.pair_mode=0U;
        ++g_link.pair_success_count;
        Indicators_Stop();
        last_send=Board_Millis()-LINK_INTERVAL_MS;
    } else if(PairingTx_Expire(Board_Millis())) PairTimeout();
}
/** @brief 等待匹配确认：1=收到 ACK，0=超时灯灭，-1=底层硬件错误。 */
static int SendDataAndWaitAck(const uint8_t *packet,const LinkMessage *sent)
{
    uint32_t started;
    uint8_t reply_packet[LINK_PACKET_SIZE];
    LinkMessage reply;
    int status;
    if(!Radio_Send(packet) || !Radio_StartReceive()) return -1;
    started=Board_Millis();
    do {
        status=Radio_PollReceive(reply_packet);
        if(status<0) return -1;
        if(status>0 && Link_DecodeData(reply_packet,&reply,TxPeer_Key()) &&
           reply.type==LINK_DATA_ACK && reply.payload==LINK_PAIR_ACK_OK &&
           reply.sender_id==sent->receiver_id && reply.receiver_id==sent->sender_id &&
           reply.session==sent->session &&
           reply.token==sent->token) return 1U;
    } while((uint32_t)(Board_Millis()-started)<LINK_ACK_TIMEOUT_MS);
    /* 没收到确认只表示本次未确认，不能关闭无线或进入红灯故障。 */
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
    TxPeer_Init();
    if(may_retry) LinkCrypto_Init(g_link.local_id,session,1);
    Indicators_Startup();
    /* Spend this scalar multiplication during boot, not after the 2s hold.
     * Ready is shown only after preparation, so no key event is lost. */
    if(may_retry) PairingTx_Prepare();
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
    int ack_result;
    Keys_Update(now,Board_Keys());
    edges=Keys_Pressed();
    if(!g_link.online) {
        Indicators_Fault(now,g_radio.error);
        if(may_retry && (uint32_t)(now-last_retry)>=5000UL) InitRadio();
        return;
    }
    if(!PairingTx_Active() && Keys_LongPress(KEY_CODE_MASK,now,2000UL)) {
        /* A fresh persistent epoch distinguishes attempts even after token wrap. */
        if(!SessionStore_Next(&session)) {
            g_radio.error=RF_STORAGE_ERROR;
            may_retry=0U;
            Fault();
            return;
        }
        g_link.session=session;
        g_link.sequence=0U;
        now=Board_Millis();
        g_link.pair_mode=1U;
        pending_keys=0U;
        Indicators_PairStart(now);
        LinkCrypto_Init(g_link.local_id,session,1);
        PairingTx_Begin(now,g_link.local_id,session,++g_link.sequence);
    }
    if(PairingTx_Active()) { PairPoll(); return; }
    pending_keys|=(uint8_t)(edges&(KEY_SW_MASK|KEY_USER_MASK));
    if(!pending_keys) return;
    if(!TxPeer_Id()) { pending_keys=0; return; }
    if((uint32_t)(now-last_send)<300UL) return;
    for(i=0;i<2U;++i) if(pending_keys&(1U<<i)) {
        command=(uint8_t)(i+1U);
        pending_keys&=(uint8_t)~(1U<<i);
        break;
    }
    message.type=LINK_DATA;
    message.session=g_link.session;
    message.sender_id=g_link.local_id;
    message.receiver_id=TxPeer_Id();
    message.payload=(uint8_t)((command<<3U)|(Board_Keys()&7U));
    if(g_link.sequence==0xffffffffUL) {
        if(!SessionStore_Next(&session)) { may_retry=0; g_radio.error=RF_STORAGE_ERROR; Fault(); return; }
        g_link.session=session; g_link.sequence=0;
        message.session=session;
    }
    message.token=++g_link.sequence;
    Link_EncodeData(packet,&message,TxPeer_Key());
    Remember(packet);
    g_link.last_command=command;
    g_link.last_keys=(uint8_t)(message.payload&7U);
    Indicators_Stop();
    ack_result=SendDataAndWaitAck(packet,&message);
    if(ack_result>0) Indicators_TxPulse(Board_Millis());
    else if(ack_result<0) Fault();
    else {
        ++g_link.ack_timeout_count;
        Indicators_Stop();
    }
    last_send=Board_Millis();
}
