/* 文件说明：圆板对码状态机：切换对码状态、保存设备、延时准备确认回包。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#include "pairing_rx.h"
#include "pair_store.h"
static uint8_t active,ready,pending;
static uint32_t entered,received;
static LinkMessage request;
/** @brief 初始化圆板对码状态和待发送确认状态。 */
void PairingRx_Init(void) { active=ready=pending=0U; }
/** @brief 切换圆板对码状态，进入后等待两秒提示音结束。 */
uint8_t PairingRx_Toggle(uint32_t now)
{
    active=(uint8_t)!active;
    ready=pending=0U;
    entered=now;
    return active;
}
/** @brief 查询圆板是否处于对码状态。 */
uint8_t PairingRx_Active(void) { return active; }
/** @brief 对码窗口有效时保存设备 ID，并排队准备确认。 */
uint8_t PairingRx_OnRequest(uint32_t now,const LinkMessage *message)
{
    if(active && !ready && (uint32_t)(now-entered)>=2000UL) ready=1U;
    if(!active || !ready || pending || message->type!=LINK_PAIR_REQUEST ||
       message->payload!=0U || !message->session ||
       !PairStore_Add(message->sender_id)) return 0U;
    request=*message;
    received=now;
    pending=1U;
    return 1U;
}
/** @brief 等待收发切换间隔后输出确认包，不使用忙等待。 */
uint8_t PairingRx_GetReply(uint32_t now,LinkMessage *reply)
{
    if(!active || !pending || (uint32_t)(now-received)<30UL) return 0U;
    *reply=request;
    reply->type=LINK_PAIR_ACK;
    reply->payload=LINK_PAIR_ACK_OK;
    return 1U;
}
/** @brief 确认回包发送后记录事务，返回是否需要首次成功提示。 */
uint8_t PairingRx_ReplySent(void)
{
    uint8_t first;
    if(!pending) return 0U;
    first=PairStore_Confirm(&request);
    pending=0U;
    return first;
}
/** @brief 清除待回确认，避免故障恢复后发送过期回复。 */
void PairingRx_CancelReply(void) { pending=0U; }
