/* 文件说明：板级驱动：初始化时钟、GPIO、SPI、按键以及灯或蜂鸣器。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef LINK_BOARD_H
#define LINK_BOARD_H
#include "stm32l1xx.h"
#include "radio_port.h"
#include "board_profile.h"
#include "sound.h"

#define RADIO_SPI SPI1
#define RADIO_SCLK_PIN GPIO_Pin_5
#define RADIO_MISO_PIN GPIO_Pin_6
#define RADIO_MOSI_PIN GPIO_Pin_7
#define RADIO_SDN_PIN GPIO_Pin_1
#define LED_MASK_RED 1U
#define LED_MASK_GREEN 2U
#define LED_MASK_WHITE 4U
#define LED_MASK_BLUE 8U
#define LED_MASK_YELLOW 16U

/** @brief 初始化当前板型的时钟、按键、提示器件和毫秒时基。 */
uint8_t Board_Init(void);
/** @brief 按位掩码设置方板 LED；未装灯的板型不执行硬件输出。 */
void Board_Leds(uint8_t mask);
/** @brief 读取实际安装的按键，按 SWITCH/USER/CODE 返回对应位。 */
uint8_t Board_Keys(void);
/** @brief 读取 MCU 唯一标识并生成当前协议使用的设备 ID。 */
uint32_t Board_DeviceId(void);
/** @brief 在短临界区内启动提示音，避免与 SysTick 同时修改声音状态。 */
void Board_Sound(uint8_t event,uint8_t code);
#endif
