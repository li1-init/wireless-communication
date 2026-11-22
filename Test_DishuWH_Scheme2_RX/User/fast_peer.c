/**
 * @file fast_peer.c
 * @brief 已配对对端信息的读取、校验和保存实现。
 */
#include "fast_peer.h"
#include "nv_store.h"
#include "link_protocol.h"

#define FAST_PEER_BASE 0x08080700UL
#define FAST_PEER_MAGIC 0x35524950UL

/**
 * @brief 已配对对端的运行状态。
 */
typedef struct
{
    uint32_t peer_id;
    uint32_t binding_id;
    uint32_t generation;
    uint8_t slot;
} FastPeerState;

static FastPeerState peer_state;

/**
 * @brief 读取并校验一个对端存储槽。
 * @param slot_index 存储槽编号。
 * @param record 输出的六字记录。
 * @return 记录有效时返回 1，否则返回 0。
 */
static uint8_t Read(uint8_t slot_index, uint32_t record[6])
{
    uint32_t address;

    address = FAST_PEER_BASE + (uint32_t)slot_index * 24UL;
    Nv_ReadBytes(address, (uint8_t *)record, 24U);

    return (uint8_t)(
        record[0] == FAST_PEER_MAGIC &&
        record[1] &&
        record[1] != 0xffffffffUL &&
        record[2] &&
        record[3] &&
        record[4] == ~record[3] &&
        record[5] == Link_Crc16((const uint8_t *)record, 20U));
}

/**
 * @brief 从非易失存储加载已绑定的对端信息。
 */
void FastPeer_Init(void)
{
    uint32_t first_record[6];
    uint32_t second_record[6];
    uint32_t *selected_record;
    uint8_t first_valid;
    uint8_t second_valid;

    selected_record = 0;
    first_valid = Read(0U, first_record);
    second_valid = Read(1U, second_record);

    peer_state.peer_id = 0U;
    peer_state.binding_id = 0U;
    peer_state.generation = 0U;
    peer_state.slot = 0U;

    if (second_valid && (!first_valid ||
                         second_record[3] > first_record[3]))
    {
        selected_record = second_record;
        peer_state.slot = 1U;
    }
    else if (first_valid)
    {
        selected_record = first_record;
    }

    if (selected_record)
    {
        peer_state.peer_id = selected_record[1];
        peer_state.binding_id = selected_record[2];
        peer_state.generation = selected_record[3];
    }
}

/**
 * @brief 获取已绑定的对端设备 ID。
 * @return 对端设备 ID，未绑定时返回 0。
 */
uint32_t FastPeer_Id(void)
{
    return peer_state.peer_id;
}

/**
 * @brief 获取当前绑定标识。
 * @return 绑定标识，未绑定时返回 0。
 */
uint32_t FastPeer_Binding(void)
{
    return peer_state.binding_id;
}

/**
 * @brief 判断收到的消息是否来自当前绑定对端。
 * @param message 待检查的消息。
 * @param local 本机设备 ID。
 * @return 消息可接受时返回 1，否则返回 0。
 */
uint8_t FastPeer_Accepts(const LinkMessage *message, uint32_t local)
{
    return (uint8_t)(
        peer_state.peer_id &&
        message->sender_id == peer_state.peer_id &&
        message->receiver_id == local &&
        message->binding == peer_state.binding_id);
}

/**
 * @brief 保存对端设备 ID 和绑定标识。
 * @param peer 对端设备 ID。
 * @param binding 绑定标识。
 * @return 保存并校验成功时返回 1，否则返回 0。
 */
uint8_t FastPeer_Save(uint32_t peer, uint32_t binding)
{
    uint32_t record[6];
    uint32_t zero;
    uint32_t address;

    zero = 0U;
    address = FAST_PEER_BASE + (uint32_t)(!peer_state.slot) * 24UL;

    if (!peer ||
        peer == 0xffffffffUL ||
        !binding ||
        peer_state.generation >= 0xfffffffeUL)
    {
        return 0U;
    }

    if (peer == peer_state.peer_id &&
        binding == peer_state.binding_id)
    {
        return 1U;
    }

    record[0] = FAST_PEER_MAGIC;
    record[1] = peer;
    record[2] = binding;
    record[3] = peer_state.generation + 1U;
    record[4] = ~record[3];
    record[5] = Link_Crc16((const uint8_t *)record, 20U);

    if (!Nv_WriteBytes(address, (const uint8_t *)&zero, 4U) ||
        !Nv_WriteBytes(address + 4U, (const uint8_t *)(record + 1), 20U) ||
        !Nv_WriteBytes(address, (const uint8_t *)record, 4U))
    {
        return 0U;
    }

    peer_state.peer_id = peer;
    peer_state.binding_id = binding;
    ++peer_state.generation;
    peer_state.slot = (uint8_t)!peer_state.slot;

    return 1U;
}
