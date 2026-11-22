/* 文件说明：软件环形包队列：保存完整数据包，管理入队、出队及包长度和内存范围查询。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This module queues complete wireless packets without crossing DMA slots. */
#include "packet_ring.h"
/** @brief 清空队列索引、长度和统计信息，不需要清零每个数据字节。 */
void PacketRing_Init(PacketRing *ring)
{
    uint8_t i;
    ring->head=ring->tail=ring->count=0U;
    ring->last_size=ring->last_start=ring->last_end=0U;
    ring->last_read_size=ring->last_read_start=ring->last_read_end=0U;
    ring->overflow=0U;
    for(i=0;i<PACKET_RING_SLOTS;++i) ring->lengths[i]=0U;
}
/** @brief 预留下一写入槽；满时返回空指针并增加溢出计数。 */
uint8_t *PacketRing_Reserve(PacketRing *ring)
{
    if(ring->count>=PACKET_RING_SLOTS) { ++ring->overflow; return 0; }
    return ring->data[ring->head];
}
/** @brief DMA 或复制完成后发布一包，记录长度并推进写位置。 */
uint8_t PacketRing_Commit(PacketRing *ring,uint16_t size)
{
    if(!size || size>LINK_PACKET_SIZE || ring->count>=PACKET_RING_SLOTS) return 0U;
    ring->lengths[ring->head]=size;
    ring->last_size=size;
    ring->last_start=(uint16_t)(ring->head*LINK_PACKET_SIZE);
    ring->last_end=(uint16_t)(ring->last_start+size);
    ring->head=(uint16_t)((ring->head+1U)%PACKET_RING_SLOTS);
    ++ring->count;
    return 1U;
}
/** @brief 把外部数据复制到空槽并提交入队，失败时不覆盖未读数据。 */
uint8_t PacketRing_Push(PacketRing *ring,const uint8_t *packet,uint16_t size)
{
    uint16_t i;
    uint8_t *slot;
    if(!packet || !size || size>LINK_PACKET_SIZE) return 0U;
    slot=PacketRing_Reserve(ring);
    if(!slot) return 0U;
    for(i=0;i<size;++i) slot[i]=packet[i];
    return PacketRing_Commit(ring,size);
}
/** @brief 返回队首数据地址但不出队；空队列返回空指针。 */
const uint8_t *PacketRing_Peek(const PacketRing *ring)
{
    return ring->count ? ring->data[ring->tail] : 0;
}
/** @brief 记录最近出队包的边界并推进读位置，释放该槽位。 */
void PacketRing_Pop(PacketRing *ring)
{
    if(!ring->count) return;
    ring->last_read_size=ring->lengths[ring->tail];
    ring->last_read_start=(uint16_t)(ring->tail*LINK_PACKET_SIZE);
    ring->last_read_end=(uint16_t)(ring->last_read_start+ring->last_read_size);
    ring->tail=(uint16_t)((ring->tail+1U)%PACKET_RING_SLOTS);
    --ring->count;
}
/** @brief 复制队首完整包到调用者缓冲区并出队，返回长度。 */
uint16_t PacketRing_Read(PacketRing *ring,uint8_t *packet)
{
    uint16_t i,size;
    const uint8_t *data=PacketRing_Peek(ring);
    if(!data || !packet) return 0U;
    size=ring->lengths[ring->tail];
    for(i=0;i<size;++i) packet[i]=data[i];
    PacketRing_Pop(ring);
    return size;
}
/** @brief 复制最近入队的包用于查看；地址内容随后可能被复用。 */
uint16_t PacketRing_CopyLast(const PacketRing *ring,uint8_t *packet,uint16_t capacity)
{
    uint16_t i;
    const uint8_t *data=ring->data[ring->last_start/LINK_PACKET_SIZE];
    if(!packet || !ring->last_size || capacity<ring->last_size) return 0U;
    for(i=0;i<ring->last_size;++i) packet[i]=data[i];
    return ring->last_size;
}
/** @brief 返回容量、排队数量、读写位置及最近包范围；结束偏移不包含该位置。 */
void PacketRing_GetInfo(const PacketRing *ring,PacketRingInfo *info)
{
    uint16_t i,index;
    if(!info) return;
    info->base_address=(uint32_t)(uintptr_t)&ring->data[0][0];
    info->capacity=PACKET_RING_BYTES;
    info->read_offset=ring->tail*LINK_PACKET_SIZE;
    info->write_offset=ring->head*LINK_PACKET_SIZE;
    info->queued_packets=ring->count;
    info->queued_bytes=0U;
    for(i=0;i<ring->count;++i) {
        index=(uint16_t)((ring->tail+i)%PACKET_RING_SLOTS);
        info->queued_bytes+=ring->lengths[index];
    }
    info->last_size=ring->last_size;
    info->last_start=ring->last_start;
    info->last_end=ring->last_end;
    info->last_start_address=info->last_size ? info->base_address+info->last_start : 0U;
    info->last_end_address=info->last_size ? info->base_address+info->last_end : 0U;
    info->overflow_count=ring->overflow;
    info->last_read_size=ring->last_read_size;
    info->last_read_start=ring->last_read_start;
    info->last_read_end=ring->last_read_end;
    info->last_read_start_address=info->last_read_size ? info->base_address+info->last_read_start : 0U;
    info->last_read_end_address=info->last_read_size ? info->base_address+info->last_read_end : 0U;
}
