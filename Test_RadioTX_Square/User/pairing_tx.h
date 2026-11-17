/* 文件说明：方板对码状态机：生成对码请求、匹配确认和处理十秒超时。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef PAIRING_TX_H
#define PAIRING_TX_H
#include "link_protocol.h"
#define PAIR_TX_TIMEOUT_MS 10000UL
/** @brief 清空方板对码活动状态。 */
void PairingTx_Init(void);
/** @brief 建立新对码事务，记录目标字段和十秒计时起点。 */
void PairingTx_Begin(uint32_t now,uint32_t id,uint32_t session,uint8_t token);
/** @brief 查询方板当前是否处于对码尝试中。 */
uint8_t PairingTx_Active(void);
/** @brief 检查十秒截止时间；到期结束事务并返回 1。 */
uint8_t PairingTx_Expire(uint32_t now);
/** @brief 达到重发间隔时输出对码请求，超时后不再产生请求。 */
uint8_t PairingTx_Request(uint32_t now,LinkMessage *message);
/** @brief 核对确认类型、ID、代号和 token，并拒绝超时确认。 */
uint8_t PairingTx_AcceptAck(uint32_t now,const LinkMessage *message);
/** @brief 中止方板当前对码事务。 */
void PairingTx_Abort(void);
#endif
