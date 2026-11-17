/* 文件说明：方板对码状态机：生成对码请求、匹配确认和处理十秒超时。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#include "pairing_tx.h"
static uint8_t active;
static uint32_t started,last_request;
static LinkMessage request;
/** @brief 清空方板对码活动状态。 */
void PairingTx_Init(void) { active=0U; }
/** @brief 建立新对码事务，记录目标字段和十秒计时起点。 */
void PairingTx_Begin(uint32_t now,uint32_t id,uint32_t session,uint8_t token)
{
    started=now;
    last_request=now-400UL;
    request.type=LINK_PAIR_REQUEST;
    request.payload=0U;
    request.sender_id=id;
    request.session=session;
    request.token=token;
    active=1U;
}
/** @brief 查询方板当前是否处于对码尝试中。 */
uint8_t PairingTx_Active(void) { return active; }
/** @brief 检查十秒截止时间；到期结束事务并返回 1。 */
uint8_t PairingTx_Expire(uint32_t now)
{
    if(active && (uint32_t)(now-started)>=PAIR_TX_TIMEOUT_MS) {
        active=0U;
        return 1U;
    }
    return 0U;
}
/** @brief 达到重发间隔时输出对码请求，超时后不再产生请求。 */
uint8_t PairingTx_Request(uint32_t now,LinkMessage *message)
{
    if(!active || (uint32_t)(now-started)>=PAIR_TX_TIMEOUT_MS ||
       (uint32_t)(now-last_request)<400UL) return 0U;
    last_request=now;
    *message=request;
    return 1U;
}
/** @brief 核对确认类型、ID、代号和 token，并拒绝超时确认。 */
uint8_t PairingTx_AcceptAck(uint32_t now,const LinkMessage *message)
{
    if(!active || (uint32_t)(now-started)>=PAIR_TX_TIMEOUT_MS ||
       message->type!=LINK_PAIR_ACK || message->payload!=LINK_PAIR_ACK_OK ||
       message->sender_id!=request.sender_id || message->session!=request.session ||
       message->token!=request.token) return 0U;
    active=0U;
    return 1U;
}
/** @brief 中止方板当前对码事务。 */
void PairingTx_Abort(void) { active=0U; }
