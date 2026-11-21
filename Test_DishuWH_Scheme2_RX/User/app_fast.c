/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 引入本模块所需的头文件接口。 */
#include "app_fast.h"
/* 引入本模块所需的头文件接口。 */
#include "board.h"
/* 引入本模块所需的头文件接口。 */
#include "key_input.h"
/* 引入本模块所需的头文件接口。 */
#include "radio_link.h"
/* 引入本模块所需的头文件接口。 */
#include "indicators.h"
/* 引入本模块所需的头文件接口。 */
#include "session_store.h"
/* 引入本模块所需的头文件接口。 */
#include "fast_pair.h"
/* 引入本模块所需的头文件接口。 */
#include "fast_peer.h"
/* 定义或初始化本模块使用的状态数据。 */
volatile FastStatus g_link;
/* 执行当前语句。 */
static uint8_t clock_ok,waiting,sent_key;
/* 执行当前语句。 */
static uint32_t last_retry,rx_session,rx_token;
/* 根据编译条件选择对应实现。 */
#if BOARD_IS_TX
/* 定义或初始化本模块使用的状态数据。 */
static uint8_t pending_keys;
/* 定义或初始化本模块使用的状态数据。 */
static uint32_t sent_at;
/* 根据编译条件选择对应实现。 */
#endif
/* 定义或初始化本模块使用的状态数据。 */
static uint8_t rx_seen;
/* 定义或初始化本模块使用的状态数据。 */
static LinkMessage sent;
/* 进入本模块的函数实现。 */
static void InitRadio(void)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    g_link.online=(uint8_t)(Radio_Init()&&Radio_StartReceive());
/* 更新运行状态或计算结果。 */
    last_retry=Board_Millis();
/* 进入本模块的函数实现。 */
    if(g_link.online) {
/* 根据运行条件选择处理分支。 */
        if(g_link.pair_mode)Indicators_PairStart(last_retry);
/* 执行条件不满足时的备用分支。 */
        else Indicators_Stop();
/* 结束当前代码块。 */
    }
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
static void Fault(void)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    Radio_Shutdown();g_link.online=0;++g_link.tx_failed;
/* 更新运行状态或计算结果。 */
    waiting=0;FastPair_Stop();g_link.pair_mode=0;Indicators_Stop();
/* 更新运行状态或计算结果。 */
    last_retry=Board_Millis();
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
static uint8_t Send(const LinkMessage *m)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t packet[LINK_PACKET_SIZE];
/* 调用相关模块接口完成当前处理步骤。 */
    Link_Encode(packet,m);
/* 进入本模块的函数实现。 */
    if(!Radio_Send(packet)||!Radio_StartReceive()){Fault();return 0;}
/* 返回当前函数的处理结果。 */
    return 1;
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
void AppFast_Init(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t epoch=0;
/* 更新运行状态或计算结果。 */
    clock_ok=Board_Init();g_link.local_id=Board_DeviceId();
/* 调用相关模块接口完成当前处理步骤。 */
    Keys_Init();FastPeer_Init();g_link.peer_id=FastPeer_Id();
/* 调用相关模块接口完成当前处理步骤。 */
    FastPair_Init(BOARD_IS_TX,g_link.local_id);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Startup();
/* 根据运行条件选择处理分支。 */
    if(!clock_ok)g_radio.error=RF_CLOCK_ERROR;
/* 进入本模块的函数实现。 */
    else if(!SessionStore_Next(&epoch)){clock_ok=0;g_radio.error=RF_STORAGE_ERROR;}
/* 执行条件不满足时的备用分支。 */
    else {g_link.session=epoch;InitRadio();}
/* 根据运行条件选择处理分支。 */
    if(g_link.online)Indicators_Ready();
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
void AppFast_Poll(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t now=Board_Millis(),epoch;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t packet[LINK_PACKET_SIZE],result;
/* 根据编译条件选择对应实现。 */
#if BOARD_IS_TX
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t edges;
/* 根据编译条件选择对应实现。 */
#endif
/* 执行当前语句。 */
    LinkMessage m,reply;
/* 定义或初始化本模块使用的状态数据。 */
    int status;
/* 调用相关模块接口完成当前处理步骤。 */
    Keys_Update(now,Board_Keys());
/* 根据编译条件选择对应实现。 */
#if BOARD_IS_TX
/* 更新运行状态或计算结果。 */
    edges=Keys_Pressed();
/* 根据编译条件选择对应实现。 */
#endif
/* 进入本模块的函数实现。 */
    if(!g_link.online) {
/* 调用相关模块接口完成当前处理步骤。 */
        Indicators_Fault(now,g_radio.error);
/* 根据运行条件选择处理分支。 */
        if(clock_ok&&(uint32_t)(now-last_retry)>=5000UL)InitRadio();
/* 返回当前函数的处理结果。 */
        return;
/* 结束当前代码块。 */
    }
/* 进入本模块的函数实现。 */
    if(Keys_LongPress(KEY_CODE_MASK,now,2000UL)) {
/* 进入本模块的函数实现。 */
        if(!BOARD_IS_TX&&FastPair_Active()) {
/* 调用相关模块接口完成当前处理步骤。 */
            FastPair_Stop();g_link.pair_mode=0;Indicators_Stop();
/* 说明本行代码在当前模块中的处理作用。 */
        } else if(!FastPair_Active()) {
            /* Show entry on threshold, without requiring release. */
/* 调用相关模块接口完成当前处理步骤。 */
            Indicators_PairStart(now);
/* 进入本模块的函数实现。 */
            if(!SessionStore_Next(&epoch)) {
/* 更新运行状态或计算结果。 */
                clock_ok=0;g_radio.error=RF_STORAGE_ERROR;Fault();return;
/* 结束当前代码块。 */
            }
/* 执行当前语句。 */
            g_link.session=epoch;g_link.sequence=0;
/* 更新运行状态或计算结果。 */
            waiting=0;
/* 根据编译条件选择对应实现。 */
#if BOARD_IS_TX
/* 更新运行状态或计算结果。 */
            pending_keys=0;
/* 更新运行状态或计算结果。 */
            sent_key=0;
/* 根据编译条件选择对应实现。 */
#endif
/* 调用相关模块接口完成当前处理步骤。 */
            FastPair_Begin(Board_Millis(),epoch);g_link.pair_mode=1;
/* 结束当前代码块。 */
        }
/* 结束当前代码块。 */
    }
/* 更新运行状态或计算结果。 */
    now=Board_Millis();
/* 进入本模块的函数实现。 */
    if(FastPair_Expired(now)) {
/* 更新运行状态或计算结果。 */
        ++g_link.pair_timeout_count;g_link.pair_mode=0;Indicators_Stop();
/* 结束当前代码块。 */
    }
    /* Receive before retrying, so an already-arrived ACK is never cleared by TX. */
/* 更新运行状态或计算结果。 */
    status=Radio_PollReceive(packet);
/* 进入本模块的函数实现。 */
    if(status<0){Fault();return;}
/* 进入本模块的函数实现。 */
    if(status>0) {
/* 更新运行状态或计算结果。 */
        now=Board_Millis();
/* 根据运行条件选择处理分支。 */
        if(!Link_Decode(packet,&m))++g_link.rx_rejected_count;
/* 进入本模块的函数实现。 */
        else if(FastPair_Active()) {
/* 更新运行状态或计算结果。 */
            result=FastPair_Input(now,&m);
/* 进入本模块的函数实现。 */
            if(result) {
/* 更新运行状态或计算结果。 */
                ++g_link.pair_success_count;g_link.peer_id=FastPeer_Id();
/* 更新运行状态或计算结果。 */
                rx_seen=0;
/* 进入本模块的函数实现。 */
                if(BOARD_IS_TX){g_link.pair_mode=0;Indicators_Stop();}
/* 执行条件不满足时的备用分支。 */
                else Indicators_PairSuccess(now);
/* 结束当前代码块。 */
            }
/* 说明本行代码在当前模块中的处理作用。 */
        } else if(!FastPeer_Accepts(&m,g_link.local_id))
/* 更新运行状态或计算结果。 */
            ++g_link.rx_rejected_count;
/* 进入本模块的函数实现。 */
        else if(BOARD_IS_TX) {
/* 进入本模块的函数实现。 */
            if(waiting&&m.type==LINK_DATA_ACK&&m.session==sent.session&&m.token==sent.token) {
/* 更新运行状态或计算结果。 */
                waiting=0;++g_link.ack_count;Indicators_KeyAck(now,sent_key);
/* 执行当前语句。 */
            } else ++g_link.rx_rejected_count;
/* 说明本行代码在当前模块中的处理作用。 */
        } else if(m.type==LINK_DATA) {
/* 定义或初始化本模块使用的状态数据。 */
            uint8_t fresh=(uint8_t)(!rx_seen||m.session>rx_session||
/* 执行当前语句。 */
                (m.session==rx_session&&m.token>rx_token));
/* 定义或初始化本模块使用的状态数据。 */
            uint8_t duplicate=(uint8_t)(rx_seen&&m.session==rx_session&&m.token==rx_token);
/* 进入本模块的函数实现。 */
            if(fresh) {
/* 定义或初始化本模块使用的状态数据。 */
                uint8_t command=(uint8_t)(m.payload>>3);
/* 更新运行状态或计算结果。 */
                rx_seen=1;rx_session=m.session;rx_token=m.token;
/* 更新运行状态或计算结果。 */
                ++g_link.rx_valid_count;
/* 执行当前语句。 */
                g_link.last_command=command;g_link.last_keys=(uint8_t)(m.payload&7);
/* 根据运行条件选择处理分支。 */
                if(command>=3U&&command<=7U)Indicators_CommandAck(now,command);
/* 执行条件不满足时的备用分支。 */
                else Indicators_RxPulse(now);
/* 结束当前代码块。 */
            }
/* 进入本模块的函数实现。 */
            if(fresh||duplicate) {
/* 更新运行状态或计算结果。 */
                reply=m;reply.type=LINK_DATA_ACK;reply.payload=1;
/* 执行当前语句。 */
                reply.sender_id=g_link.local_id;reply.receiver_id=m.sender_id;
/* 调用相关模块接口完成当前处理步骤。 */
                Board_DelayMs(15);if(!Send(&reply))return;
/* 执行当前语句。 */
            } else ++g_link.rx_rejected_count;
/* 执行当前语句。 */
        } else ++g_link.rx_rejected_count;
/* 结束当前代码块。 */
    }
/* 进入本模块的函数实现。 */
    if(FastPair_Active()) {
/* 根据运行条件选择处理分支。 */
        if(FastPair_Output(Board_Millis(),&reply))(void)Send(&reply);
/* 返回当前函数的处理结果。 */
        return;
/* 结束当前代码块。 */
    }
/* 根据编译条件选择对应实现。 */
#if BOARD_IS_TX
/* 说明本行代码在当前模块中的处理作用。 */
    pending_keys|=(uint8_t)(edges&(KEY_SW_MASK|KEY_USER_MASK|
/* 执行当前语句。 */
        KEY_W1_MASK|KEY_W2_MASK|KEY_W3_MASK|KEY_W4_MASK|KEY_W5_MASK));
/* 进入本模块的函数实现。 */
    if(waiting) {
/* 进入本模块的函数实现。 */
        if((uint32_t)(Board_Millis()-sent_at)>=250UL) {
/* 更新运行状态或计算结果。 */
            waiting=0;++g_link.ack_timeout_count;Indicators_Stop();
/* 结束当前代码块。 */
        }
/* 返回当前函数的处理结果。 */
        return;
/* 结束当前代码块。 */
    }
/* 根据运行条件选择处理分支。 */
    if(!pending_keys)return;
/* 进入本模块的函数实现。 */
    if(!FastPeer_Id()){pending_keys=0;return;}
/* 进入本模块的函数实现。 */
    if(g_link.sequence==0xffffffffUL) {
/* 进入本模块的函数实现。 */
        if(!SessionStore_Next(&epoch)){clock_ok=0;g_radio.error=RF_STORAGE_ERROR;Fault();return;}
/* 执行当前语句。 */
        g_link.session=epoch;g_link.sequence=0;
/* 结束当前代码块。 */
    }
/* 执行当前语句。 */
    sent.type=LINK_DATA;sent.sender_id=g_link.local_id;sent.receiver_id=FastPeer_Id();
/* 执行当前语句。 */
    sent.binding=FastPeer_Binding();sent.session=g_link.session;sent.token=++g_link.sequence;
/* 开始当前代码块。 */
    {
/* 定义或初始化本模块使用的状态数据。 */
        uint8_t command=1U;
/* 更新运行状态或计算结果。 */
        sent_key=KEY_SW_MASK;
/* 进入本模块的函数实现。 */
        if(pending_keys&KEY_USER_MASK) {sent_key=KEY_USER_MASK;command=2U;}
/* 进入本模块的函数实现。 */
        else if(pending_keys&KEY_W1_MASK) {sent_key=KEY_W1_MASK;command=3U;}
/* 进入本模块的函数实现。 */
        else if(pending_keys&KEY_W2_MASK) {sent_key=KEY_W2_MASK;command=4U;}
/* 进入本模块的函数实现。 */
        else if(pending_keys&KEY_W3_MASK) {sent_key=KEY_W3_MASK;command=5U;}
/* 进入本模块的函数实现。 */
        else if(pending_keys&KEY_W4_MASK) {sent_key=KEY_W4_MASK;command=6U;}
/* 进入本模块的函数实现。 */
        else if(pending_keys&KEY_W5_MASK) {sent_key=KEY_W5_MASK;command=7U;}
/* 执行当前语句。 */
        sent.payload=(uint8_t)(command<<3);
/* 结束当前代码块。 */
    }
/* 执行当前语句。 */
    sent.payload|=(uint8_t)(Board_Keys()&7U);
/* 执行当前语句。 */
    pending_keys&=(uint8_t)~sent_key;
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Stop();
/* 进入本模块的函数实现。 */
    if(Send(&sent)){sent_at=Board_Millis();waiting=1;}
/* 根据编译条件选择对应实现。 */
#endif
/* 结束当前代码块。 */
}
