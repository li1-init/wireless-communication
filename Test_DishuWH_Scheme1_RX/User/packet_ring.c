/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：软件环形包队列：保存完整数据包，管理入队、出队及包长度和内存范围查询。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 引入本模块所需的头文件接口。 */
#include "packet_ring.h"
/** @brief 清空队列索引、长度和统计信息，不需要清零每个数据字节。 */
/* 进入本模块的函数实现。 */
void PacketRing_Init(PacketRing *ring)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t i;
/* 更新运行状态或计算结果。 */
    ring->head=ring->tail=ring->count=0U;
/* 更新运行状态或计算结果。 */
    ring->last_size=ring->last_start=ring->last_end=0U;
/* 更新运行状态或计算结果。 */
    ring->last_read_size=ring->last_read_start=ring->last_read_end=0U;
/* 更新运行状态或计算结果。 */
    ring->overflow=0U;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<PACKET_RING_SLOTS;++i) ring->lengths[i]=0U;
/* 结束当前代码块。 */
}
/** @brief 预留下一写入槽；满时返回空指针并增加溢出计数。 */
/* 进入本模块的函数实现。 */
uint8_t *PacketRing_Reserve(PacketRing *ring)
/* 开始当前代码块。 */
{
/* 进入本模块的函数实现。 */
    if(ring->count>=PACKET_RING_SLOTS) { ++ring->overflow; return 0; }
/* 返回当前函数的处理结果。 */
    return ring->data[ring->head];
/* 结束当前代码块。 */
}
/** @brief DMA 或复制完成后发布一包，记录长度并推进写位置。 */
/* 进入本模块的函数实现。 */
uint8_t PacketRing_Commit(PacketRing *ring,uint16_t size)
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(!size || size>LINK_PACKET_SIZE || ring->count>=PACKET_RING_SLOTS) return 0U;
/* 执行当前语句。 */
    ring->lengths[ring->head]=size;
/* 更新运行状态或计算结果。 */
    ring->last_size=size;
/* 更新运行状态或计算结果。 */
    ring->last_start=(uint16_t)(ring->head*LINK_PACKET_SIZE);
/* 更新运行状态或计算结果。 */
    ring->last_end=(uint16_t)(ring->last_start+size);
/* 更新运行状态或计算结果。 */
    ring->head=(uint16_t)((ring->head+1U)%PACKET_RING_SLOTS);
/* 更新运行状态或计算结果。 */
    ++ring->count;
/* 返回当前函数的处理结果。 */
    return 1U;
/* 结束当前代码块。 */
}
/** @brief 把外部数据复制到空槽并提交入队，失败时不覆盖未读数据。 */
/* 进入本模块的函数实现。 */
uint8_t PacketRing_Push(PacketRing *ring,const uint8_t *packet,uint16_t size)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t i;
/* 执行当前语句。 */
    uint8_t *slot;
/* 根据运行条件选择处理分支。 */
    if(!packet || !size || size>LINK_PACKET_SIZE) return 0U;
/* 更新运行状态或计算结果。 */
    slot=PacketRing_Reserve(ring);
/* 根据运行条件选择处理分支。 */
    if(!slot) return 0U;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<size;++i) slot[i]=packet[i];
/* 返回当前函数的处理结果。 */
    return PacketRing_Commit(ring,size);
/* 结束当前代码块。 */
}
/** @brief 返回队首数据地址但不出队；空队列返回空指针。 */
/* 进入本模块的函数实现。 */
const uint8_t *PacketRing_Peek(const PacketRing *ring)
/* 开始当前代码块。 */
{
/* 返回当前函数的处理结果。 */
    return ring->count ? ring->data[ring->tail] : 0;
/* 结束当前代码块。 */
}
/** @brief 记录最近出队包的边界并推进读位置，释放该槽位。 */
/* 进入本模块的函数实现。 */
void PacketRing_Pop(PacketRing *ring)
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(!ring->count) return;
/* 更新运行状态或计算结果。 */
    ring->last_read_size=ring->lengths[ring->tail];
/* 更新运行状态或计算结果。 */
    ring->last_read_start=(uint16_t)(ring->tail*LINK_PACKET_SIZE);
/* 更新运行状态或计算结果。 */
    ring->last_read_end=(uint16_t)(ring->last_read_start+ring->last_read_size);
/* 更新运行状态或计算结果。 */
    ring->tail=(uint16_t)((ring->tail+1U)%PACKET_RING_SLOTS);
/* 更新运行状态或计算结果。 */
    --ring->count;
/* 结束当前代码块。 */
}
/** @brief 复制队首完整包到调用者缓冲区并出队，返回长度。 */
/* 进入本模块的函数实现。 */
uint16_t PacketRing_Read(PacketRing *ring,uint8_t *packet)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    uint16_t i,size;
/* 执行当前语句。 */
    const uint8_t *data=PacketRing_Peek(ring);
/* 根据运行条件选择处理分支。 */
    if(!data || !packet) return 0U;
/* 更新运行状态或计算结果。 */
    size=ring->lengths[ring->tail];
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<size;++i) packet[i]=data[i];
/* 调用相关模块接口完成当前处理步骤。 */
    PacketRing_Pop(ring);
/* 返回当前函数的处理结果。 */
    return size;
/* 结束当前代码块。 */
}
/** @brief 复制最近入队的包用于查看；地址内容随后可能被复用。 */
/* 进入本模块的函数实现。 */
uint16_t PacketRing_CopyLast(const PacketRing *ring,uint8_t *packet,uint16_t capacity)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t i;
/* 执行当前语句。 */
    const uint8_t *data=ring->data[ring->last_start/LINK_PACKET_SIZE];
/* 根据运行条件选择处理分支。 */
    if(!packet || !ring->last_size || capacity<ring->last_size) return 0U;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<ring->last_size;++i) packet[i]=data[i];
/* 返回当前函数的处理结果。 */
    return ring->last_size;
/* 结束当前代码块。 */
}
/** @brief 返回容量、排队数量、读写位置及最近包范围；结束偏移不包含该位置。 */
/* 进入本模块的函数实现。 */
void PacketRing_GetInfo(const PacketRing *ring,PacketRingInfo *info)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    uint16_t i,index;
/* 根据运行条件选择处理分支。 */
    if(!info) return;
/* 更新运行状态或计算结果。 */
    info->base_address=(uint32_t)(uintptr_t)&ring->data[0][0];
/* 更新运行状态或计算结果。 */
    info->capacity=PACKET_RING_BYTES;
/* 更新运行状态或计算结果。 */
    info->read_offset=ring->tail*LINK_PACKET_SIZE;
/* 更新运行状态或计算结果。 */
    info->write_offset=ring->head*LINK_PACKET_SIZE;
/* 更新运行状态或计算结果。 */
    info->queued_packets=ring->count;
/* 更新运行状态或计算结果。 */
    info->queued_bytes=0U;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<ring->count;++i) {
/* 更新运行状态或计算结果。 */
        index=(uint16_t)((ring->tail+i)%PACKET_RING_SLOTS);
/* 执行当前语句。 */
        info->queued_bytes+=ring->lengths[index];
/* 结束当前代码块。 */
    }
/* 更新运行状态或计算结果。 */
    info->last_size=ring->last_size;
/* 更新运行状态或计算结果。 */
    info->last_start=ring->last_start;
/* 更新运行状态或计算结果。 */
    info->last_end=ring->last_end;
/* 更新运行状态或计算结果。 */
    info->last_start_address=info->last_size ? info->base_address+info->last_start : 0U;
/* 更新运行状态或计算结果。 */
    info->last_end_address=info->last_size ? info->base_address+info->last_end : 0U;
/* 更新运行状态或计算结果。 */
    info->overflow_count=ring->overflow;
/* 更新运行状态或计算结果。 */
    info->last_read_size=ring->last_read_size;
/* 更新运行状态或计算结果。 */
    info->last_read_start=ring->last_read_start;
/* 更新运行状态或计算结果。 */
    info->last_read_end=ring->last_read_end;
/* 更新运行状态或计算结果。 */
    info->last_read_start_address=info->last_read_size ? info->base_address+info->last_read_start : 0U;
/* 更新运行状态或计算结果。 */
    info->last_read_end_address=info->last_read_size ? info->base_address+info->last_read_end : 0U;
/* 结束当前代码块。 */
}
