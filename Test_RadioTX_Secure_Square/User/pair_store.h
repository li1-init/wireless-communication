/* 文件说明：接收端持久化链路名单：发送端 ID、32 字节链路密钥和防重放状态。
 * 注释范围：自编应用代码；第三方 TweetNaCl 保持原始实现。
 */
#ifndef PAIR_STORE_H
#define PAIR_STORE_H
#include <stdint.h>
#include "link_protocol.h"
typedef struct {
    uint32_t id;
    uint8_t key[LINK_KEY_BYTES];
    uint32_t last_session,last_token,generation;
    uint8_t seen,valid;
} PairEntry;
/** @brief 从 EEPROM 双记录恢复唯一的当前对端。 */
void PairStore_Init(void);
/** @brief 按发送端 ID 查找运行名单槽位，未找到返回 -1。 */
int PairStore_Find(uint32_t id);
/** @brief 取得已配对发送端的链路密钥。 */
uint8_t PairStore_GetKey(uint32_t id,uint8_t key[LINK_KEY_BYTES]);
/** @brief 持久保存一条新的链路密钥；满额或写失败返回 0。 */
uint8_t PairStore_AddLink(uint32_t id,const uint8_t key[LINK_KEY_BYTES]);
/** @brief 返回指定运行槽位。 */
PairEntry *PairStore_Get(int slot);
/** @brief 检查发送端、session 和 token 防重放，然后记录本次合法事务。 */
uint8_t PairStore_AcceptData(const LinkMessage *message);
#endif
