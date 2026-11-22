/* 文件说明：按键模块：消抖、检测按下边沿，并保证每次长按只触发一次。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef KEY_INPUT_H
#define KEY_INPUT_H
#include <stdint.h>
#define KEY_SW_MASK 1U
#define KEY_USER_MASK 2U
#define KEY_CODE_MASK 4U
/** @brief 清空按键消抖及长按锁存状态。 */
void Keys_Init(void);
/** @brief 更新按键原始状态和稳定状态，生成本轮按下事件。 */
void Keys_Update(uint32_t now,uint8_t raw);
/** @brief 取得本轮消抖后的按下边沿位掩码。 */
uint8_t Keys_Pressed(void);
/** @brief 检测达到阈值的长按，同一次按住只触发一次。 */
uint8_t Keys_LongPress(uint8_t mask,uint32_t now,uint32_t threshold);
#endif
