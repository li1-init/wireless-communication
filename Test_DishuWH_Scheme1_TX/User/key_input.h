/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：按键模块：消抖、检测按下边沿，并保证每次长按只触发一次。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 根据编译条件选择对应实现。 */
#ifndef KEY_INPUT_H
/* 定义编译期常量或接口宏。 */
#define KEY_INPUT_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/* 定义编译期常量或接口宏。 */
#define KEY_SW_MASK 1U
/* 定义编译期常量或接口宏。 */
#define KEY_USER_MASK 2U
/* 定义编译期常量或接口宏。 */
#define KEY_CODE_MASK 4U
/* 定义编译期常量或接口宏。 */
#define KEY_W1_MASK 8U
/* 定义编译期常量或接口宏。 */
#define KEY_W2_MASK 16U
/* 定义编译期常量或接口宏。 */
#define KEY_W3_MASK 32U
/* 定义编译期常量或接口宏。 */
#define KEY_W4_MASK 64U
/* 定义编译期常量或接口宏。 */
#define KEY_W5_MASK 128U
/** @brief 清空按键消抖及长按锁存状态。 */
/* 执行当前语句。 */
void Keys_Init(void);
/** @brief 更新按键原始状态和稳定状态，生成本轮按下事件。 */
/* 执行当前语句。 */
void Keys_Update(uint32_t now,uint8_t raw);
/** @brief 取得本轮消抖后的按下边沿位掩码。 */
/* 执行当前语句。 */
uint8_t Keys_Pressed(void);
/** @brief 检测达到阈值的长按，同一次按住只触发一次。 */
/* 执行当前语句。 */
uint8_t Keys_LongPress(uint8_t mask,uint32_t now,uint32_t threshold);
/* 根据编译条件选择对应实现。 */
#endif
