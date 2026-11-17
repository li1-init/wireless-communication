/* 文件说明：圆板对码状态机：切换对码状态、保存设备、延时准备确认回包。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef PAIRING_RX_H
#define PAIRING_RX_H
#include "link_protocol.h"
/** @brief 初始化圆板对码状态和待发送确认状态。 */
void PairingRx_Init(void);
/** @brief 切换圆板对码状态，进入后等待两秒提示音结束。 */
uint8_t PairingRx_Toggle(uint32_t now);
/** @brief 查询圆板是否处于对码状态。 */
uint8_t PairingRx_Active(void);
/** @brief 对码窗口有效时保存设备 ID，并排队准备确认。 */
uint8_t PairingRx_OnRequest(uint32_t now,const LinkMessage *request);
/** @brief 等待收发切换间隔后输出确认包，不使用忙等待。 */
uint8_t PairingRx_GetReply(uint32_t now,LinkMessage *reply);
/** @brief 确认回包发送后记录事务，返回是否需要首次成功提示。 */
uint8_t PairingRx_ReplySent(void);
/** @brief 清除待回确认，避免故障恢复后发送过期回复。 */
void PairingRx_CancelReply(void);
#endif
