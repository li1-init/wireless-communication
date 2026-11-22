/* 文件说明：配对名单模块：掉电保存最多八个发送端 ID，并维护每台设备的对码状态。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This file declares the receiver's persistent multi-transmitter ID list. */
#ifndef PAIR_STORE_H
#define PAIR_STORE_H
#include <stdint.h>
#include "link_protocol.h"
typedef struct {
    uint32_t id,session;
    uint8_t sequence,seen;
    uint32_t pair_session;
    uint8_t pair_token,pair_seen;
} PairEntry;
/** @brief 从 EEPROM 重建配对名单；有效记录保留，运行状态重新初始化。 */
void PairStore_Init(void);
/** @brief 按发送端 ID 查找槽位，未找到返回 -1。 */
int PairStore_Find(uint32_t id);
/** @brief 添加或确认已有 ID；重复不占新槽，满额或写失败返回 0。 */
uint8_t PairStore_Add(uint32_t id);
/** @brief 返回指定有效槽位地址，越界返回空指针。 */
PairEntry *PairStore_Get(int slot);
/** @brief 判断普通包发送端是否已配对，不要求历史序号递增。 */
uint8_t PairStore_AcceptData(const LinkMessage *message);
/** @brief 按设备记录对码事务，首次成功返回 1，重复事务返回 0。 */
uint8_t PairStore_Confirm(const LinkMessage *request);
#endif
