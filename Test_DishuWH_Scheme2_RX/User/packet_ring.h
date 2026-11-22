/**
 * @file packet_ring.h
 * @brief 软件环形包队列接口。
 */
#ifndef PACKET_RING_H
#define PACKET_RING_H

#include <stdint.h>
#include "link_protocol.h"

#define PACKET_RING_SLOTS 8U
#define PACKET_RING_BYTES (PACKET_RING_SLOTS*LINK_PACKET_SIZE)

/**
 * @brief 环形包队列的可观察统计信息。
 */
typedef struct
{
    uint32_t base_address;
    uint32_t capacity;
    uint32_t read_offset;
    uint32_t write_offset;
    uint32_t queued_packets;
    uint32_t queued_bytes;
    uint32_t last_size;
    uint32_t last_start;
    uint32_t last_end;
    uint32_t last_start_address;
    uint32_t last_end_address;
    uint32_t overflow_count;
    uint32_t last_read_size;
    uint32_t last_read_start;
    uint32_t last_read_end;
    uint32_t last_read_start_address;
    uint32_t last_read_end_address;
} PacketRingInfo;

/**
 * @brief 环形包队列的运行数据。
 */
typedef struct
{
    uint8_t data[PACKET_RING_SLOTS][LINK_PACKET_SIZE];
    uint16_t lengths[PACKET_RING_SLOTS];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint16_t last_size;
    uint16_t last_start;
    uint16_t last_end;
    uint16_t last_read_size;
    uint16_t last_read_start;
    uint16_t last_read_end;
    uint32_t overflow;
} PacketRing;

/**
 * @brief 清空队列索引、长度和统计信息。
 * @param ring 要初始化的队列。
 */
void PacketRing_Init(PacketRing *ring);

/**
 * @brief 预留下一写入槽。
 * @param ring 目标队列。
 * @return 可写入的槽地址，队列已满时返回空指针。
 */
uint8_t *PacketRing_Reserve(PacketRing *ring);

/**
 * @brief 提交最近预留的槽并推进写位置。
 * @param ring 目标队列。
 * @param size 当前数据包长度。
 * @return 提交成功时返回 1，否则返回 0。
 */
uint8_t PacketRing_Commit(PacketRing *ring, uint16_t size);

/**
 * @brief 复制外部数据到空槽并提交入队。
 * @param ring 目标队列。
 * @param packet 待复制的数据。
 * @param size 数据长度。
 * @return 入队成功时返回 1，否则返回 0。
 */
uint8_t PacketRing_Push(PacketRing *ring,
                        const uint8_t *packet,
                        uint16_t size);

/**
 * @brief 查看队首数据但不出队。
 * @param ring 目标队列。
 * @return 队首数据地址，队列为空时返回空指针。
 */
const uint8_t *PacketRing_Peek(const PacketRing *ring);

/**
 * @brief 释放队首槽并推进读位置。
 * @param ring 目标队列。
 */
void PacketRing_Pop(PacketRing *ring);

/**
 * @brief 复制队首数据到调用者缓冲区并出队。
 * @param ring 目标队列。
 * @param packet 输出缓冲区。
 * @return 复制的数据长度。
 */
uint16_t PacketRing_Read(PacketRing *ring, uint8_t *packet);

/**
 * @brief 复制最近入队的数据包。
 * @param ring 目标队列。
 * @param packet 输出缓冲区。
 * @param capacity 输出缓冲区容量。
 * @return 复制的数据长度。
 */
uint16_t PacketRing_CopyLast(const PacketRing *ring,
                             uint8_t *packet,
                             uint16_t capacity);

/**
 * @brief 获取队列容量、读写位置和最近数据范围。
 * @param ring 目标队列。
 * @param info 输出的统计信息。
 */
void PacketRing_GetInfo(const PacketRing *ring, PacketRingInfo *info);

#endif
