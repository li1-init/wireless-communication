/**
 * @file key_input.h
 * @brief 按键扫描、消抖和长按检测接口。
 */
#ifndef KEY_INPUT_H
#define KEY_INPUT_H

#include <stdint.h>

/** 按键扫描支持的按键数量。 */
#define KEY_INPUT_COUNT 8U

/** SWITCH 按键对应的位掩码。 */
#define KEY_SW_MASK 1U

/** USER 按键对应的位掩码。 */
#define KEY_USER_MASK 2U

/** CODE 按键对应的位掩码。 */
#define KEY_CODE_MASK 4U

/** W1 按键对应的位掩码。 */
#define KEY_W1_MASK 8U

/** W2 按键对应的位掩码。 */
#define KEY_W2_MASK 16U

/** W3 按键对应的位掩码。 */
#define KEY_W3_MASK 32U

/** W4 按键对应的位掩码。 */
#define KEY_W4_MASK 64U

/** W5 按键对应的位掩码。 */
#define KEY_W5_MASK 128U

/**
 * @brief 初始化按键消抖和长按检测状态。
 */
void Keys_Init(void);

/**
 * @brief 扫描一次按键并更新消抖后的稳定状态。
 * @param now 当前系统毫秒时间戳，通常来自 Board_Millis()。
 * @param raw 按键原始 GPIO 电平，每一位对应一路按键。
 */
void Keys_Update(uint32_t now, uint8_t raw);

/**
 * @brief 读取本次扫描新确认的按下边沿。
 * @return 按键按下边沿位掩码。
 */
uint8_t Keys_Pressed(void);

/**
 * @brief 检查指定按键是否达到长按时间。
 * @param mask 要检查的按键位掩码。
 * @param now 当前系统毫秒时间戳。
 * @param threshold 长按判定时间，单位为毫秒。
 * @return 本次达到长按阈值的按键位掩码。
 */
uint8_t Keys_LongPress(uint8_t mask, uint32_t now, uint32_t threshold);

#endif
