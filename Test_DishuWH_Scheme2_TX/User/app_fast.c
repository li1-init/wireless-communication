/* V5 fast addressed link. No cryptography; exactly one current peer per board. */
#include "app_fast.h"
#include "board.h"
#include "key_input.h"
#include "radio_link.h"
#include "indicators.h"
#include "session_store.h"
#include "fast_pair.h"
#include "fast_peer.h"
volatile FastStatus g_link;
static uint8_t clock_ok,waiting,sent_key;
static uint32_t last_retry,rx_session,rx_token;
#if BOARD_IS_TX
static uint8_t pending_keys;
static uint32_t sent_at;
#endif
static uint8_t rx_seen;
static LinkMessage sent;
static void InitRadio(void)
{
    g_link.online=(uint8_t)(Radio_Init()&&Radio_StartReceive());
    last_retry=Board_Millis();
    if(g_link.online) {
        if(g_link.pair_mode)Indicators_PairStart(last_retry);
        else Indicators_Stop();
    }
}
static void Fault(void)
{
    Radio_Shutdown();g_link.online=0;++g_link.tx_failed;
    waiting=0;FastPair_Stop();g_link.pair_mode=0;Indicators_Stop();
    last_retry=Board_Millis();
}
static uint8_t Send(const LinkMessage *m)
{
    uint8_t packet[LINK_PACKET_SIZE];
    Link_Encode(packet,m);
    if(!Radio_Send(packet)||!Radio_StartReceive()){Fault();return 0;}
    return 1;
}
void AppFast_Init(void)
{
    uint32_t epoch=0;
    clock_ok=Board_Init();g_link.local_id=Board_DeviceId();
    Keys_Init();FastPeer_Init();g_link.peer_id=FastPeer_Id();
    FastPair_Init(BOARD_IS_TX,g_link.local_id);
    Indicators_Startup();
    if(!clock_ok)g_radio.error=RF_CLOCK_ERROR;
    else if(!SessionStore_Next(&epoch)){clock_ok=0;g_radio.error=RF_STORAGE_ERROR;}
    else {g_link.session=epoch;InitRadio();}
    if(g_link.online)Indicators_Ready();
}
void AppFast_Poll(void)
{
    uint32_t now=Board_Millis(),epoch;
    uint8_t packet[LINK_PACKET_SIZE],result;
#if BOARD_IS_TX
    uint8_t edges;
#endif
    LinkMessage m,reply;
    int status;
    Keys_Update(now,Board_Keys());
#if BOARD_IS_TX
    edges=Keys_Pressed();
#endif
    if(!g_link.online) {
        Indicators_Fault(now,g_radio.error);
        if(clock_ok&&(uint32_t)(now-last_retry)>=5000UL)InitRadio();
        return;
    }
    if(Keys_LongPress(KEY_CODE_MASK,now,2000UL)) {
        if(!BOARD_IS_TX&&FastPair_Active()) {
            FastPair_Stop();g_link.pair_mode=0;Indicators_Stop();
        } else if(!FastPair_Active()) {
            /* Show entry on threshold, without requiring release. */
            Indicators_PairStart(now);
            if(!SessionStore_Next(&epoch)) {
                clock_ok=0;g_radio.error=RF_STORAGE_ERROR;Fault();return;
            }
            g_link.session=epoch;g_link.sequence=0;
            waiting=0;
#if BOARD_IS_TX
            pending_keys=0;
            sent_key=0;
#endif
            FastPair_Begin(Board_Millis(),epoch);g_link.pair_mode=1;
        }
    }
    now=Board_Millis();
    if(FastPair_Expired(now)) {
        ++g_link.pair_timeout_count;g_link.pair_mode=0;Indicators_Stop();
    }
    /* Receive before retrying, so an already-arrived ACK is never cleared by TX. */
    status=Radio_PollReceive(packet);
    if(status<0){Fault();return;}
    if(status>0) {
        now=Board_Millis();
        if(!Link_Decode(packet,&m))++g_link.rx_rejected_count;
        else if(FastPair_Active()) {
            result=FastPair_Input(now,&m);
            if(result) {
                ++g_link.pair_success_count;g_link.peer_id=FastPeer_Id();
                rx_seen=0;
                if(BOARD_IS_TX){g_link.pair_mode=0;Indicators_Stop();}
                else Indicators_PairSuccess(now);
            }
        } else if(!FastPeer_Accepts(&m,g_link.local_id))
            ++g_link.rx_rejected_count;
        else if(BOARD_IS_TX) {
            if(waiting&&m.type==LINK_DATA_ACK&&m.session==sent.session&&m.token==sent.token) {
                waiting=0;++g_link.ack_count;Indicators_KeyAck(now,sent_key);
            } else ++g_link.rx_rejected_count;
        } else if(m.type==LINK_DATA) {
            uint8_t fresh=(uint8_t)(!rx_seen||m.session>rx_session||
                (m.session==rx_session&&m.token>rx_token));
            uint8_t duplicate=(uint8_t)(rx_seen&&m.session==rx_session&&m.token==rx_token);
            if(fresh) {
                rx_seen=1;rx_session=m.session;rx_token=m.token;
                ++g_link.rx_valid_count;
                g_link.last_command=(uint8_t)(m.payload>>3);g_link.last_keys=(uint8_t)(m.payload&7);
                Indicators_RxPulse(now);
            }
            if(fresh||duplicate) {
                reply=m;reply.type=LINK_DATA_ACK;reply.payload=1;
                reply.sender_id=g_link.local_id;reply.receiver_id=m.sender_id;
                Board_DelayMs(15);if(!Send(&reply))return;
            } else ++g_link.rx_rejected_count;
        } else ++g_link.rx_rejected_count;
    }
    if(FastPair_Active()) {
        if(FastPair_Output(Board_Millis(),&reply))(void)Send(&reply);
        return;
    }
#if BOARD_IS_TX
    pending_keys|=(uint8_t)(edges&(KEY_SW_MASK|KEY_USER_MASK|
        KEY_W1_MASK|KEY_W2_MASK|KEY_W3_MASK|KEY_W4_MASK|KEY_W5_MASK));
    if(waiting) {
        if((uint32_t)(Board_Millis()-sent_at)>=250UL) {
            waiting=0;++g_link.ack_timeout_count;Indicators_Stop();
        }
        return;
    }
    if(!pending_keys)return;
    if(!FastPeer_Id()){pending_keys=0;return;}
    if(g_link.sequence==0xffffffffUL) {
        if(!SessionStore_Next(&epoch)){clock_ok=0;g_radio.error=RF_STORAGE_ERROR;Fault();return;}
        g_link.session=epoch;g_link.sequence=0;
    }
    sent.type=LINK_DATA;sent.sender_id=g_link.local_id;sent.receiver_id=FastPeer_Id();
    sent.binding=FastPeer_Binding();sent.session=g_link.session;sent.token=++g_link.sequence;
    {
        uint8_t command=1U;
        sent_key=KEY_SW_MASK;
        if(pending_keys&KEY_USER_MASK) {sent_key=KEY_USER_MASK;command=2U;}
        else if(pending_keys&KEY_W1_MASK) {sent_key=KEY_W1_MASK;command=3U;}
        else if(pending_keys&KEY_W2_MASK) {sent_key=KEY_W2_MASK;command=4U;}
        else if(pending_keys&KEY_W3_MASK) {sent_key=KEY_W3_MASK;command=5U;}
        else if(pending_keys&KEY_W4_MASK) {sent_key=KEY_W4_MASK;command=6U;}
        else if(pending_keys&KEY_W5_MASK) {sent_key=KEY_W5_MASK;command=7U;}
        sent.payload=(uint8_t)(command<<3);
    }
    sent.payload|=(uint8_t)(Board_Keys()&7U);
    pending_keys&=(uint8_t)~sent_key;
    Indicators_Stop();
    if(Send(&sent)){sent_at=Board_Millis();waiting=1;}
#endif
}
