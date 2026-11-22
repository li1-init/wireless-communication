/**
 * @file app_fast.c
 * @brief 快速配对、链路收发和应用轮询实现。
 */
#include "app_fast.h"
#include "board.h"
#include "key_input.h"
#include "radio_link.h"
#include "indicators.h"
#include "session_store.h"
#include "fast_pair.h"
#include "fast_peer.h"

volatile FastStatus g_link;

/**
 * @brief 应用轮询所需的内部运行状态。
 */
typedef struct
{
    uint8_t clock_ok;
    uint8_t waiting;
    uint8_t sent_key;
    uint8_t rx_seen;
    uint32_t last_retry;
    uint32_t rx_session;
    uint32_t rx_token;
    LinkMessage sent;
#if BOARD_IS_TX
    uint8_t pending_keys;
    uint32_t sent_at;
#endif
} AppFastState;

static AppFastState app_state;

/**
 * @brief 初始化无线芯片并进入接收状态。
 */
static void InitRadio(void)
{
    g_link.runtime.online =
        (uint8_t)(Radio_Init() && Radio_StartReceive());
    app_state.last_retry = Board_Millis();

    if (g_link.runtime.online)
    {
        if (g_link.runtime.pair_mode)
        {
            Indicators_PairStart(app_state.last_retry);
        }
        else
        {
            Indicators_Stop();
        }
    }
}

/**
 * @brief 处理无线或存储故障并清理当前链路状态。
 */
static void Fault(void)
{
    Radio_Shutdown();
    g_link.runtime.online = 0U;
    ++g_link.counters.tx_failed;

    app_state.waiting = 0U;
    FastPair_Stop();
    g_link.runtime.pair_mode = 0U;
    Indicators_Stop();
    app_state.last_retry = Board_Millis();
}

/**
 * @brief 编码并发送一帧，然后重新进入接收状态。
 * @param message 要发送的链路消息。
 * @return 发送和重新进入接收状态均成功时返回 1，否则返回 0。
 */
static uint8_t Send(const LinkMessage *message)
{
    uint8_t packet[LINK_PACKET_SIZE];

    Link_Encode(packet, message);

    if (!Radio_Send(packet) || !Radio_StartReceive())
    {
        Fault();
        return 0U;
    }

    return 1U;
}

/**
 * @brief 初始化板级资源、持久化会话号和无线链路。
 */
void AppFast_Init(void)
{
    uint32_t epoch;

    epoch = 0U;
    app_state.clock_ok = Board_Init();
    g_link.identity.local_id = Board_DeviceId();

    Keys_Init();
    FastPeer_Init();
    g_link.identity.peer_id = FastPeer_Id();
    FastPair_Init(BOARD_IS_TX, g_link.identity.local_id);
    Indicators_Startup();

    if (!app_state.clock_ok)
    {
        g_radio.error = RF_CLOCK_ERROR;
    }
    else if (!SessionStore_Next(&epoch))
    {
        app_state.clock_ok = 0U;
        g_radio.error = RF_STORAGE_ERROR;
    }
    else
    {
        g_link.identity.session = epoch;
        InitRadio();
    }

    if (g_link.runtime.online)
    {
        Indicators_Ready();
    }
}

/**
 * @brief 执行一次按键、配对、无线收发和指示灯轮询。
 */
void AppFast_Poll(void)
{
    uint32_t now;
    uint32_t epoch;
    uint8_t packet[LINK_PACKET_SIZE];
    uint8_t result;
    LinkMessage message;
    LinkMessage reply;
    int status;
#if BOARD_IS_TX
    uint8_t edges;
#endif

    now = Board_Millis();
    epoch = 0U;
    result = 0U;
    status = 0;

    Keys_Update(now, Board_Keys());

#if BOARD_IS_TX
    edges = Keys_Pressed();
#endif

    if (!g_link.runtime.online)
    {
        Indicators_Fault(now, g_radio.error);

        if (app_state.clock_ok &&
            (uint32_t)(now - app_state.last_retry) >= 5000UL)
        {
            InitRadio();
        }

        return;
    }

    if (Keys_LongPress(KEY_CODE_MASK, now, 2000UL))
    {
        if (!BOARD_IS_TX && FastPair_Active())
        {
            FastPair_Stop();
            g_link.runtime.pair_mode = 0U;
            Indicators_Stop();
        }
        else if (!FastPair_Active())
        {
            Indicators_PairStart(now);

            if (!SessionStore_Next(&epoch))
            {
                app_state.clock_ok = 0U;
                g_radio.error = RF_STORAGE_ERROR;
                Fault();
                return;
            }

            g_link.identity.session = epoch;
            g_link.identity.sequence = 0U;
            app_state.waiting = 0U;
#if BOARD_IS_TX
            app_state.pending_keys = 0U;
            app_state.sent_key = 0U;
#endif
            FastPair_Begin(Board_Millis(), epoch);
            g_link.runtime.pair_mode = 1U;
        }
    }

    now = Board_Millis();

    if (FastPair_Expired(now))
    {
        ++g_link.counters.pair_timeout_count;
        g_link.runtime.pair_mode = 0U;
        Indicators_Stop();
    }

    status = Radio_PollReceive(packet);

    if (status < 0)
    {
        Fault();
        return;
    }

    if (status > 0)
    {
        now = Board_Millis();

        if (!Link_Decode(packet, &message))
        {
            ++g_link.counters.rx_rejected_count;
        }
        else if (FastPair_Active())
        {
            result = FastPair_Input(now, &message);

            if (result)
            {
                ++g_link.counters.pair_success_count;
                g_link.identity.peer_id = FastPeer_Id();
                app_state.rx_seen = 0U;

                if (BOARD_IS_TX)
                {
                    g_link.runtime.pair_mode = 0U;
                    Indicators_Stop();
                }
                else
                {
                    Indicators_PairSuccess(now);
                }
            }
        }
        else if (!FastPeer_Accepts(&message, g_link.identity.local_id))
        {
            ++g_link.counters.rx_rejected_count;
        }
        else if (BOARD_IS_TX)
        {
            if (app_state.waiting &&
                message.type == LINK_DATA_ACK &&
                message.session == app_state.sent.session &&
                message.token == app_state.sent.token)
            {
                app_state.waiting = 0U;
                ++g_link.counters.ack_count;
                Indicators_KeyAck(now, app_state.sent_key);
            }
            else
            {
                ++g_link.counters.rx_rejected_count;
            }
        }
        else if (message.type == LINK_DATA)
        {
            uint8_t fresh;
            uint8_t duplicate;
            uint8_t command;

            fresh = (uint8_t)(
                !app_state.rx_seen ||
                message.session > app_state.rx_session ||
                (message.session == app_state.rx_session &&
                 message.token > app_state.rx_token));
            duplicate = (uint8_t)(
                app_state.rx_seen &&
                message.session == app_state.rx_session &&
                message.token == app_state.rx_token);

            if (fresh)
            {
                command = (uint8_t)(message.payload >> 3);
                app_state.rx_seen = 1U;
                app_state.rx_session = message.session;
                app_state.rx_token = message.token;
                ++g_link.counters.rx_valid_count;
                g_link.runtime.last_command = command;
                g_link.runtime.last_keys = (uint8_t)(message.payload & 7U);

                if (command >= 3U && command <= 7U)
                {
                    Indicators_CommandAck(now, command);
                }
                else
                {
                    Indicators_RxPulse(now);
                }
            }

            if (fresh || duplicate)
            {
                reply = message;
                reply.type = LINK_DATA_ACK;
                reply.payload = 1U;
                reply.sender_id = g_link.identity.local_id;
                reply.receiver_id = message.sender_id;
                Board_DelayMs(15U);

                if (!Send(&reply))
                {
                    return;
                }
            }
            else
            {
                ++g_link.counters.rx_rejected_count;
            }
        }
        else
        {
            ++g_link.counters.rx_rejected_count;
        }
    }

    if (FastPair_Active())
    {
        if (FastPair_Output(Board_Millis(), &reply))
        {
            (void)Send(&reply);
        }

        return;
    }

#if BOARD_IS_TX
    app_state.pending_keys |=
        (uint8_t)(edges & (KEY_SW_MASK |
                           KEY_USER_MASK |
                           KEY_W1_MASK |
                           KEY_W2_MASK |
                           KEY_W3_MASK |
                           KEY_W4_MASK |
                           KEY_W5_MASK));

    if (app_state.waiting)
    {
        if ((uint32_t)(Board_Millis() - app_state.sent_at) >= 250UL)
        {
            app_state.waiting = 0U;
            ++g_link.counters.ack_timeout_count;
            Indicators_Stop();
        }

        return;
    }

    if (!app_state.pending_keys)
    {
        return;
    }

    if (!FastPeer_Id())
    {
        app_state.pending_keys = 0U;
        return;
    }

    if (g_link.identity.sequence == 0xffffffffUL)
    {
        if (!SessionStore_Next(&epoch))
        {
            app_state.clock_ok = 0U;
            g_radio.error = RF_STORAGE_ERROR;
            Fault();
            return;
        }

        g_link.identity.session = epoch;
        g_link.identity.sequence = 0U;
    }

    app_state.sent.type = LINK_DATA;
    app_state.sent.sender_id = g_link.identity.local_id;
    app_state.sent.receiver_id = FastPeer_Id();
    app_state.sent.binding = FastPeer_Binding();
    app_state.sent.session = g_link.identity.session;
    app_state.sent.token = ++g_link.identity.sequence;

    {
        uint8_t command;

        command = 1U;
        app_state.sent_key = KEY_SW_MASK;

        if (app_state.pending_keys & KEY_USER_MASK)
        {
            app_state.sent_key = KEY_USER_MASK;
            command = 2U;
        }
        else if (app_state.pending_keys & KEY_W1_MASK)
        {
            app_state.sent_key = KEY_W1_MASK;
            command = 3U;
        }
        else if (app_state.pending_keys & KEY_W2_MASK)
        {
            app_state.sent_key = KEY_W2_MASK;
            command = 4U;
        }
        else if (app_state.pending_keys & KEY_W3_MASK)
        {
            app_state.sent_key = KEY_W3_MASK;
            command = 5U;
        }
        else if (app_state.pending_keys & KEY_W4_MASK)
        {
            app_state.sent_key = KEY_W4_MASK;
            command = 6U;
        }
        else if (app_state.pending_keys & KEY_W5_MASK)
        {
            app_state.sent_key = KEY_W5_MASK;
            command = 7U;
        }

        app_state.sent.payload = (uint8_t)(command << 3);
    }

    app_state.sent.payload |= (uint8_t)(Board_Keys() & 7U);
    app_state.pending_keys &= (uint8_t)~app_state.sent_key;
    Indicators_Stop();

    if (Send(&app_state.sent))
    {
        app_state.sent_at = Board_Millis();
        app_state.waiting = 1U;
    }
#endif
}
