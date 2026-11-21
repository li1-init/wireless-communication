/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 根据编译条件选择对应实现。 */
#ifndef TEST_BOARD_STUB_H
/* 定义编译期常量或接口宏。 */
#define TEST_BOARD_STUB_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/* 定义编译期常量或接口宏。 */
#define LINK_BOARD_H
/* 定义编译期常量或接口宏。 */
#define LED_MASK_RED 1U
/* 定义编译期常量或接口宏。 */
#define LED_MASK_GREEN 2U
/* 定义编译期常量或接口宏。 */
#define LED_MASK_WHITE 4U
/* 定义编译期常量或接口宏。 */
#define LED_MASK_BLUE 8U
/* 定义编译期常量或接口宏。 */
#define LED_MASK_YELLOW 16U
/* 进入本模块的函数实现。 */
static inline uint32_t __get_PRIMASK(void){return 0;}
/* 进入本模块的函数实现。 */
static inline void __disable_irq(void){}
/* 进入本模块的函数实现。 */
static inline void __set_PRIMASK(uint32_t v){(void)v;}
/* 执行当前语句。 */
void Board_Leds(uint8_t mask);
/* 执行当前语句。 */
void Board_DelayMs(uint32_t ms);
/* 执行当前语句。 */
uint8_t Board_Init(void);
/* 执行当前语句。 */
uint32_t Board_Millis(void);
/* 执行当前语句。 */
uint32_t Board_DeviceId(void);
/* 执行当前语句。 */
uint8_t Board_Keys(void);
/* 根据编译条件选择对应实现。 */
#endif
