/**
 * @file packet_ring.c
 * @brief 软件环形包队列实现。
 */
#include "packet_ring.h"

/**
 * @brief 清空队列索引、长度和统计信息。
 * @param ring 要初始化的队列。
 */
void PacketRing_Init(PacketRing *ring)
{
    uint8_t index;

    ring->head = 0U;
    ring->tail = 0U;
    ring->count = 0U;
    ring->last_size = 0U;
    ring->last_start = 0U;
    ring->last_end = 0U;
    ring->last_read_size = 0U;
    ring->last_read_start = 0U;
    ring->last_read_end = 0U;
    ring->overflow = 0U;

    for (index = 0U; index < PACKET_RING_SLOTS; ++index)
    {
        ring->lengths[index] = 0U;
    }
}

/**
 * @brief 预留下一写入槽。
 * @param ring 目标队列。
 * @return 可写入的槽地址，队列已满时返回空指针。
 */
uint8_t *PacketRing_Reserve(PacketRing *ring)
{
    if (ring->count >= PACKET_RING_SLOTS)
    {
        ++ring->overflow;
        return 0;
    }

    return ring->data[ring->head];
}

/**
 * @brief 提交最近预留的槽并推进写位置。
 * @param ring 目标队列。
 * @param size 当前数据包长度。
 * @return 提交成功时返回 1，否则返回 0。
 */
uint8_t PacketRing_Commit(PacketRing *ring, uint16_t size)
{
    if (!size ||
        size > LINK_PACKET_SIZE ||
        ring->count >= PACKET_RING_SLOTS)
    {
        return 0U;
    }

    ring->lengths[ring->head] = size;
    ring->last_size = size;
    ring->last_start = (uint16_t)(ring->head * LINK_PACKET_SIZE);
    ring->last_end = (uint16_t)(ring->last_start + size);
    ring->head = (uint16_t)((ring->head + 1U) % PACKET_RING_SLOTS);
    ++ring->count;

    return 1U;
}

/**
 * @brief 复制外部数据到空槽并提交入队。
 * @param ring 目标队列。
 * @param packet 待复制的数据。
 * @param size 数据长度。
 * @return 入队成功时返回 1，否则返回 0。
 */
uint8_t PacketRing_Push(PacketRing *ring,
                        const uint8_t *packet,
                        uint16_t size)
{
    uint16_t index;
    uint8_t *slot;

    if (!packet || !size || size > LINK_PACKET_SIZE)
    {
        return 0U;
    }

    slot = PacketRing_Reserve(ring);

    if (!slot)
    {
        return 0U;
    }

    for (index = 0U; index < size; ++index)
    {
        slot[index] = packet[index];
    }

    return PacketRing_Commit(ring, size);
}

/**
 * @brief 查看队首数据但不出队。
 * @param ring 目标队列。
 * @return 队首数据地址，队列为空时返回空指针。
 */
const uint8_t *PacketRing_Peek(const PacketRing *ring)
{
    return ring->count ? ring->data[ring->tail] : 0;
}

/**
 * @brief 释放队首槽并推进读位置。
 * @param ring 目标队列。
 */
void PacketRing_Pop(PacketRing *ring)
{
    if (!ring->count)
    {
        return;
    }

    ring->last_read_size = ring->lengths[ring->tail];
    ring->last_read_start =
        (uint16_t)(ring->tail * LINK_PACKET_SIZE);
    ring->last_read_end =
        (uint16_t)(ring->last_read_start + ring->last_read_size);
    ring->tail = (uint16_t)((ring->tail + 1U) % PACKET_RING_SLOTS);
    --ring->count;
}

/**
 * @brief 复制队首数据到调用者缓冲区并出队。
 * @param ring 目标队列。
 * @param packet 输出缓冲区。
 * @return 复制的数据长度。
 */
uint16_t PacketRing_Read(PacketRing *ring, uint8_t *packet)
{
    uint16_t index;
    uint16_t size;
    const uint8_t *data;

    data = PacketRing_Peek(ring);

    if (!data || !packet)
    {
        return 0U;
    }

    size = ring->lengths[ring->tail];

    for (index = 0U; index < size; ++index)
    {
        packet[index] = data[index];
    }

    PacketRing_Pop(ring);
    return size;
}

/**
 * @brief 复制最近入队的数据包。
 * @param ring 目标队列。
 * @param packet 输出缓冲区。
 * @param capacity 输出缓冲区容量。
 * @return 复制的数据长度。
 */
uint16_t PacketRing_CopyLast(const PacketRing *ring,
                             uint8_t *packet,
                             uint16_t capacity)
{
    uint16_t index;
    const uint8_t *data;

    if (!packet || !ring->last_size || capacity < ring->last_size)
    {
        return 0U;
    }

    data = ring->data[ring->last_start / LINK_PACKET_SIZE];

    for (index = 0U; index < ring->last_size; ++index)
    {
        packet[index] = data[index];
    }

    return ring->last_size;
}

/**
 * @brief 获取队列容量、读写位置和最近数据范围。
 * @param ring 目标队列。
 * @param info 输出的统计信息。
 */
void PacketRing_GetInfo(const PacketRing *ring, PacketRingInfo *info)
{
    uint16_t index;
    uint16_t slot_index;

    if (!info)
    {
        return;
    }

    info->base_address = (uint32_t)(uintptr_t)&ring->data[0][0];
    info->capacity = PACKET_RING_BYTES;
    info->read_offset = ring->tail * LINK_PACKET_SIZE;
    info->write_offset = ring->head * LINK_PACKET_SIZE;
    info->queued_packets = ring->count;
    info->queued_bytes = 0U;

    for (index = 0U; index < ring->count; ++index)
    {
        slot_index =
            (uint16_t)((ring->tail + index) % PACKET_RING_SLOTS);
        info->queued_bytes += ring->lengths[slot_index];
    }

    info->last_size = ring->last_size;
    info->last_start = ring->last_start;
    info->last_end = ring->last_end;
    info->last_start_address =
        info->last_size ? info->base_address + info->last_start : 0U;
    info->last_end_address =
        info->last_size ? info->base_address + info->last_end : 0U;
    info->overflow_count = ring->overflow;
    info->last_read_size = ring->last_read_size;
    info->last_read_start = ring->last_read_start;
    info->last_read_end = ring->last_read_end;
    info->last_read_start_address =
        info->last_read_size ?
            info->base_address + info->last_read_start : 0U;
    info->last_read_end_address =
        info->last_read_size ?
            info->base_address + info->last_read_end : 0U;
}
