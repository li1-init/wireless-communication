/* 文件说明：配对名单模块：掉电保存最多八个发送端 ID，并维护每台设备的对码状态。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This module saves paired transmitter IDs and per-device receive state. */
#include "pair_store.h"
#include "link_protocol.h"
#include "nv_store.h"
#define PAIR_CAPACITY 8U
#define PAIR_BASE 0x08080040UL
static PairEntry paired[PAIR_CAPACITY];
static uint8_t pair_count;
/** @brief 将一个发送端 ID 持久化到指定配对槽，校验成功后更新内存名单。 */
static uint8_t Store(uint8_t slot,uint32_t id)
{
    uint32_t address=PAIR_BASE+(uint32_t)slot*8UL;
    if(!Nv_WritePair(address,id)) return 0U;
    paired[slot].id=id;
    ++pair_count;
    return 1U;
}
/** @brief 从 EEPROM 重建配对名单；有效记录保留，运行状态重新初始化。 */
void PairStore_Init(void)
{
    uint8_t i;
    uint32_t first=Nv_ReadWord(PAIR_BASE);
    uint32_t inverse=Nv_ReadWord(PAIR_BASE+4UL);
    pair_count=0U;
    for(i=0;i<PAIR_CAPACITY;++i) {
        uint32_t address=PAIR_BASE+(uint32_t)i*8UL;
        uint32_t id=Nv_ReadWord(address),inv=Nv_ReadWord(address+4UL);
        PairEntry empty={0};
        paired[i]=empty;
        if(id!=0U && id!=0xFFFFFFFFUL && inv==~id) {
            paired[i].id=id;
            ++pair_count;
        }
    }
    if(pair_count==0U && (first==0U || first==0xFFFFFFFFUL) &&
       (inverse==0U || inverse==0xFFFFFFFFUL))
        (void)Store(0U,LINK_EXPECTED_SENDER_ID);
}
/** @brief 按发送端 ID 查找槽位，未找到返回 -1。 */
int PairStore_Find(uint32_t id)
{
    uint8_t i;
    for(i=0;i<PAIR_CAPACITY;++i) if(paired[i].id==id) return i;
    return -1;
}
/** @brief 添加或确认已有 ID；重复不占新槽，满额或写失败返回 0。 */
uint8_t PairStore_Add(uint32_t id)
{
    uint8_t i;
    if(id==0U || id==0xFFFFFFFFUL) return 0U;
    if(PairStore_Find(id)>=0) return 1U;
    if(pair_count>=PAIR_CAPACITY) return 0U;
    for(i=0;i<PAIR_CAPACITY;++i) if(paired[i].id==0U) return Store(i,id);
    return 0U;
}
/** @brief 返回指定有效槽位地址，越界返回空指针。 */
PairEntry *PairStore_Get(int slot)
{
    return (slot>=0 && slot<(int)PAIR_CAPACITY) ? &paired[slot] : 0;
}
/** @brief 判断普通包发送端是否已配对，不要求历史序号递增。 */
uint8_t PairStore_AcceptData(const LinkMessage *message)
{
    PairEntry *entry=PairStore_Get(PairStore_Find(message->sender_id));
    if(message->type!=LINK_DATA || !entry) return 0U;
    entry->seen=1U;
    entry->session=message->session;
    entry->sequence=message->token;
    return 1U;
}
/** @brief 按设备记录对码事务，首次成功返回 1，重复事务返回 0。 */
uint8_t PairStore_Confirm(const LinkMessage *request)
{
    PairEntry *entry=PairStore_Get(PairStore_Find(request->sender_id));
    if(!entry) return 0U;
    if(entry->pair_seen && entry->pair_session==request->session &&
       entry->pair_token==request->token) return 0U;
    entry->pair_seen=1U;
    entry->pair_session=request->session;
    entry->pair_token=request->token;
    entry->seen=0U; /* Explicit pairing also recovers an erased TX epoch. */
    return 1U;
}
