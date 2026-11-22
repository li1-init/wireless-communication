/**
 * @file board.h
 * @brief 板级时钟、GPIO、按键、LED 和 SPI 接口。
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

/**
 * @brief 初始化当前板型的时钟、按键、提示器件和毫秒时基。
 * @return 初始化成功时返回 1，否则返回 0。
 */
uint8_t Board_Init(void);

/**
 * @brief 按位掩码设置 LED。
 * @param mask LED 位掩码。
 */
void Board_Leds(uint8_t mask);

/**
 * @brief 读取当前所有按键的原始 GPIO 电平。
 * @return 按键原始电平位掩码。
 */
uint8_t Board_Keys(void);

/**
 * @brief 读取 MCU 唯一标识并生成设备 ID。
 * @return 当前设备 ID。
 */
uint32_t Board_DeviceId(void);

/**
 * @brief 在临界区内启动提示音。
 * @param event 提示音事件类型。
 * @param code 故障码或提示音编号。
 */
void Board_Sound(uint8_t event, uint8_t code);

#endif
