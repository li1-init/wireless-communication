/**
 * @file key_input.c
 * @brief 按键扫描、消抖和长按检测实现。
 */
#include "key_input.h"

/**
 * @brief 按键模块的运行状态。
 */
typedef struct
{
    uint8_t previous;
    uint8_t stable;
    uint8_t pressed;
    uint8_t long_fired;
    uint32_t changed[KEY_INPUT_COUNT];
    uint32_t down_since[KEY_INPUT_COUNT];
} KeyState;

static KeyState key_state;

/**
 * @brief 初始化按键消抖和长按检测状态。
 */
void Keys_Init(void)
{
    uint8_t index;

    key_state.previous = 0U;
    key_state.stable = 0U;
    key_state.pressed = 0U;
    key_state.long_fired = 0U;

    for (index = 0U; index < KEY_INPUT_COUNT; ++index)
    {
        key_state.changed[index] = 0U;
        key_state.down_since[index] = 0U;
    }
}

/**
 * @brief 扫描一次按键并更新消抖后的稳定状态。
 * @param now 当前系统毫秒时间戳，通常来自 Board_Millis()。
 * @param raw 按键原始 GPIO 电平，每一位对应一路按键。
 */
void Keys_Update(uint32_t now, uint8_t raw)
{
    uint8_t index;
    uint8_t bit;

    key_state.pressed = 0U;

    for (index = 0U; index < KEY_INPUT_COUNT; ++index)
    {
        bit = (uint8_t)(1U << index);

        if ((raw ^ key_state.previous) & bit)
        {
            key_state.changed[index] = now;
        }

        if (((raw ^ key_state.stable) & bit) &&
            (uint32_t)(now - key_state.changed[index]) >= 25UL)
        {
            key_state.stable ^= bit;

            if (key_state.stable & bit)
            {
                key_state.pressed |= bit;
                key_state.down_since[index] = now;
                key_state.long_fired &= (uint8_t)~bit;
            }
        }
    }

    key_state.previous = raw;
}

/**
 * @brief 读取本次扫描新确认的按下边沿。
 * @return 按键按下边沿位掩码。
 */
uint8_t Keys_Pressed(void)
{
    return key_state.pressed;
}

/**
 * @brief 检查指定按键是否达到长按时间。
 * @param mask 要检查的按键位掩码。
 * @param now 当前系统毫秒时间戳。
 * @param threshold 长按判定时间，单位为毫秒。
 * @return 本次达到长按阈值的按键位掩码。
 */
uint8_t Keys_LongPress(uint8_t mask, uint32_t now, uint32_t threshold)
{
    uint8_t index;
    uint8_t bit;
    uint8_t result;

    result = 0U;

    for (index = 0U; index < KEY_INPUT_COUNT; ++index)
    {
        bit = (uint8_t)(1U << index);

        if ((mask & key_state.stable & bit) &&
            !(key_state.long_fired & bit) &&
            (uint32_t)(now - key_state.down_since[index]) >= threshold)
        {
            key_state.long_fired |= bit;
            result |= bit;
        }
    }

    return result;
}
