/* 文件说明：软件环形包队列：保存完整数据包，管理入队、出队及包长度和内存范围查询。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This file defines the fixed-slot packet ring and address inspection API. */
#ifndef PACKET_RING_H
#define PACKET_RING_H
#include <stdint.h>
#include "link_protocol.h"
#define PACKET_RING_SLOTS 8U
#define PACKET_RING_BYTES (PACKET_RING_SLOTS*LINK_PACKET_SIZE)
/* All offsets are bytes. End is exclusive, and can equal capacity.
 * Complete frames occupy one slot, so DMA never crosses the ring boundary. */
typedef struct {
    uint32_t base_address,capacity,read_offset,write_offset;
    uint32_t queued_packets,queued_bytes,last_size,last_start,last_end;
    uint32_t last_start_address,last_end_address,overflow_count;
    uint32_t last_read_size,last_read_start,last_read_end;
    uint32_t last_read_start_address,last_read_end_address;
} PacketRingInfo;
typedef struct {
    uint8_t data[PACKET_RING_SLOTS][LINK_PACKET_SIZE];
    uint16_t lengths[PACKET_RING_SLOTS];
    uint16_t head,tail,count,last_size,last_start,last_end;
    uint16_t last_read_size,last_read_start,last_read_end;
    uint32_t overflow;
} PacketRing;
/* Single main-loop owner, not an ISR/multi-thread queue. */
/** @brief 清空队列索引、长度和统计信息，不需要清零每个数据字节。 */
void PacketRing_Init(PacketRing *ring);
/** @brief 预留下一写入槽；满时返回空指针并增加溢出计数。 */
uint8_t *PacketRing_Reserve(PacketRing *ring);
/** @brief DMA 或复制完成后发布一包，记录长度并推进写位置。 */
uint8_t PacketRing_Commit(PacketRing *ring,uint16_t size);
/** @brief 把外部数据复制到空槽并提交入队，失败时不覆盖未读数据。 */
uint8_t PacketRing_Push(PacketRing *ring,const uint8_t *packet,uint16_t size);
/** @brief 返回队首数据地址但不出队；空队列返回空指针。 */
const uint8_t *PacketRing_Peek(const PacketRing *ring);
/** @brief 记录最近出队包的边界并推进读位置，释放该槽位。 */
void PacketRing_Pop(PacketRing *ring);
/** @brief 复制队首完整包到调用者缓冲区并出队，返回长度。 */
uint16_t PacketRing_Read(PacketRing *ring,uint8_t *packet);
/** @brief 复制最近入队的包用于查看；地址内容随后可能被复用。 */
uint16_t PacketRing_CopyLast(const PacketRing *ring,uint8_t *packet,uint16_t capacity);
/** @brief 返回容量、排队数量、读写位置及最近包范围；结束偏移不包含该位置。 */
void PacketRing_GetInfo(const PacketRing *ring,PacketRingInfo *info);
#endif
