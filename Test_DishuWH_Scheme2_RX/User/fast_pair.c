/**
 * @file fast_pair.c
 * @brief 快速配对状态机实现。
 */
#include <string.h>

#include "fast_pair.h"
#include "fast_peer.h"

/**
 * @brief 快速配对状态机的运行数据。
 */
typedef struct
{
    uint8_t transmitter;
    uint8_t active;
    uint8_t stage;
    uint8_t pending;
    uint8_t committed;
    uint8_t candidate_valid;
    uint32_t local;
    uint32_t started;
    uint32_t last_send;
    uint32_t reply_at;
    uint32_t candidate_at;
    LinkMessage transaction;
    LinkMessage queued;
} FastPairState;

static FastPairState pair_state;

/**
 * @brief 初始化快速配对状态机。
 * @param transmitter 是否为发送端。
 * @param local 本机设备 ID。
 */
void FastPair_Init(uint8_t transmitter, uint32_t local)
{
    pair_state.transmitter = transmitter;
    pair_state.local = local;
    pair_state.active = 0U;
    pair_state.stage = 0U;
    pair_state.pending = 0U;
    pair_state.committed = 0U;
    pair_state.candidate_valid = 0U;
}

/**
 * @brief 停止当前快速配对事务。
 */
void FastPair_Stop(void)
{
    pair_state.active = 0U;
    pair_state.pending = 0U;
    pair_state.candidate_valid = 0U;
    pair_state.committed = 0U;
}

/**
 * @brief 开始一次快速配对事务。
 * @param now 当前系统毫秒时间戳。
 * @param session 本次配对使用的会话号。
 */
void FastPair_Begin(uint32_t now, uint32_t session)
{
    FastPair_Stop();

    pair_state.active = 1U;
    pair_state.started = now;
    pair_state.last_send = now - 150UL;
    pair_state.stage = 0U;
    memset(&pair_state.transaction, 0, sizeof(pair_state.transaction));

    pair_state.transaction.sender_id = pair_state.local;
    pair_state.transaction.session = session;
    pair_state.transaction.token = 1U;
    pair_state.transaction.type = LINK_PAIR_REQUEST;
}

/**
 * @brief 查询快速配对是否处于活动状态。
 * @return 活动时返回 1，否则返回 0。
 */
uint8_t FastPair_Active(void)
{
    return pair_state.active;
}

/**
 * @brief 检查发送端配对事务是否超时。
 * @param now 当前系统毫秒时间戳。
 * @return 已超时时返回 1，否则返回 0。
 */
uint8_t FastPair_Expired(uint32_t now)
{
    if (pair_state.transmitter &&
        pair_state.active &&
        (uint32_t)(now - pair_state.started) >= 10000UL)
    {
        FastPair_Stop();
        return 1U;
    }

    return 0U;
}

/**
 * @brief 准备一个延迟回复。
 * @param now 当前系统毫秒时间戳。
 * @param message 要回复的消息。
 * @param type 回复消息类型。
 */
static void Reply(uint32_t now, const LinkMessage *message, uint8_t type)
{
    pair_state.queued = *message;
    pair_state.queued.type = type;
    pair_state.queued.sender_id = pair_state.local;
    pair_state.queued.receiver_id = message->sender_id;
    pair_state.queued.payload = 1U;
    pair_state.pending = 1U;
    pair_state.reply_at = now;
}

/**
 * @brief 根据当前阶段生成待发送的配对消息。
 * @param now 当前系统毫秒时间戳。
 * @param out 输出的配对消息。
 * @return 需要发送消息时返回 1，否则返回 0。
 */
uint8_t FastPair_Output(uint32_t now, LinkMessage *out)
{
    if (!pair_state.active)
    {
        return 0U;
    }

    if (pair_state.pending &&
        (uint32_t)(now - pair_state.reply_at) >= 15UL)
    {
        *out = pair_state.queued;
        pair_state.pending = 0U;
        return 1U;
    }

    if (pair_state.transmitter &&
        (uint32_t)(now - pair_state.last_send) >= 150UL)
    {
        pair_state.last_send = now;
        *out = pair_state.transaction;

        if (pair_state.stage)
        {
            out->type = LINK_PAIR_COMMIT;
            out->payload = 1U;
        }

        return 1U;
    }

    return 0U;
}

/**
 * @brief 输入一帧配对消息并推进状态机。
 * @param now 当前系统毫秒时间戳。
 * @param message 收到的配对消息。
 * @return 0 表示拒绝或未完成，1 表示接收端首次提交，2 表示发送端最终完成。
 */
uint8_t FastPair_Input(uint32_t now, const LinkMessage *message)
{
    if (!pair_state.active || message->sender_id == pair_state.local)
    {
        return 0U;
    }

    if (pair_state.transmitter)
    {
        if (message->receiver_id != pair_state.local ||
            message->session != pair_state.transaction.session ||
            message->token != pair_state.transaction.token)
        {
            return 0U;
        }

        if (!pair_state.stage &&
            message->type == LINK_PAIR_ACK &&
            message->binding == pair_state.transaction.session)
        {
            pair_state.transaction.receiver_id = message->sender_id;
            pair_state.transaction.binding = message->binding;
            pair_state.stage = 1U;
            pair_state.last_send = now - 150UL;
        }
        else if (pair_state.stage &&
                 message->type == LINK_PAIR_DONE &&
                 message->sender_id == pair_state.transaction.receiver_id &&
                 message->binding == pair_state.transaction.binding)
        {
            if (!FastPeer_Save(message->sender_id, message->binding))
            {
                return 0U;
            }

            FastPair_Stop();
            return 2U;
        }

        return 0U;
    }

    if (message->type == LINK_PAIR_REQUEST)
    {
        if (pair_state.candidate_valid &&
            (uint32_t)(now - pair_state.candidate_at) < 2000UL &&
            (message->sender_id != pair_state.transaction.sender_id ||
             message->session != pair_state.transaction.session ||
             message->token != pair_state.transaction.token))
        {
            return 0U;
        }

        if (!pair_state.candidate_valid ||
            message->sender_id != pair_state.transaction.sender_id ||
            message->session != pair_state.transaction.session ||
            message->token != pair_state.transaction.token)
        {
            pair_state.committed = 0U;
        }

        pair_state.transaction = *message;
        pair_state.transaction.binding = message->session;
        pair_state.candidate_valid = 1U;
        pair_state.candidate_at = now;
        Reply(now, &pair_state.transaction, LINK_PAIR_ACK);
        return 0U;
    }

    if (message->type != LINK_PAIR_COMMIT ||
        !pair_state.candidate_valid ||
        message->receiver_id != pair_state.local ||
        message->sender_id != pair_state.transaction.sender_id ||
        message->session != pair_state.transaction.session ||
        message->token != pair_state.transaction.token ||
        message->binding != pair_state.transaction.binding)
    {
        return 0U;
    }

    if (!pair_state.committed)
    {
        if (!FastPeer_Save(message->sender_id, message->binding))
        {
            return 0U;
        }

        pair_state.committed = 1U;
        Reply(now, message, LINK_PAIR_DONE);
        return 1U;
    }

    Reply(now, message, LINK_PAIR_DONE);
    return 0U;
}
