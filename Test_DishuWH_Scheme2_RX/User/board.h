/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：板级驱动：初始化时钟、GPIO、SPI、按键以及灯或蜂鸣器。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 根据编译条件选择对应实现。 */
#ifndef LINK_BOARD_H
/* 定义编译期常量或接口宏。 */
#define LINK_BOARD_H
/* 引入本模块所需的头文件接口。 */
#include "stm32l1xx.h"
/* 引入本模块所需的头文件接口。 */
#include "radio_port.h"
/* 引入本模块所需的头文件接口。 */
#include "board_profile.h"
/* 引入本模块所需的头文件接口。 */
#include "sound.h"

/* 定义编译期常量或接口宏。 */
#define RADIO_SPI SPI1
/* 定义编译期常量或接口宏。 */
#define RADIO_SCLK_PIN GPIO_Pin_5
/* 定义编译期常量或接口宏。 */
#define RADIO_MISO_PIN GPIO_Pin_6
/* 定义编译期常量或接口宏。 */
#define RADIO_MOSI_PIN GPIO_Pin_7
/* 定义编译期常量或接口宏。 */
#define RADIO_SDN_PIN GPIO_Pin_1
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

/** @brief 初始化当前板型的时钟、按键、提示器件和毫秒时基。 */
/* 执行当前语句。 */
uint8_t Board_Init(void);
/** @brief 按位掩码设置方板 LED；未装灯的板型不执行硬件输出。 */
/* 执行当前语句。 */
void Board_Leds(uint8_t mask);
/** @brief 读取实际安装的按键，按 SWITCH/USER/CODE 返回对应位。 */
/* 执行当前语句。 */
uint8_t Board_Keys(void);
/** @brief 读取 MCU 唯一标识并生成当前协议使用的设备 ID。 */
/* 执行当前语句。 */
uint32_t Board_DeviceId(void);
/** @brief 在短临界区内启动提示音，避免与 SysTick 同时修改声音状态。 */
/* 执行当前语句。 */
void Board_Sound(uint8_t event,uint8_t code);
/* 根据编译条件选择对应实现。 */
#endif
