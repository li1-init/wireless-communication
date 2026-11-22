/**
 * @file indicators.c
 * @brief LED 指示灯控制实现。
 */
#include "indicators.h"
#include "board.h"
#include "key_input.h"

/**
 * @brief LED 非阻塞灯效状态。
 */
typedef struct
{
    volatile uint8_t timed;
    volatile uint8_t pair_active;
    volatile uint8_t pair_flash;
    volatile uint8_t flash_steps;
    volatile uint32_t started;
    volatile uint32_t duration;
    volatile uint32_t pair_started;
} IndicatorState;

static IndicatorState indicator_state;

/**
 * @brief 设置 LED 及自动熄灭计时。
 * @param now 当前系统毫秒时间戳。
 * @param milliseconds 点亮时长，单位为毫秒。
 * @param mask LED 位掩码。
 */
static void TimedLight(uint32_t now, uint32_t milliseconds, uint8_t mask)
{
    uint32_t saved_primask;

    saved_primask = __get_PRIMASK();
    __disable_irq();
    indicator_state.timed = 1U;
    indicator_state.started = now;
    indicator_state.duration = milliseconds;
    indicator_state.pair_active = 0U;
    indicator_state.pair_flash = 0U;
    Board_Leds(mask);
    __set_PRIMASK(saved_primask);
}

/**
 * @brief 取消全部灯控状态并关闭 LED。
 */
void Indicators_Stop(void)
{
    uint32_t saved_primask;

    saved_primask = __get_PRIMASK();
    __disable_irq();
    indicator_state.timed = 0U;
    indicator_state.pair_active = 0U;
    indicator_state.pair_flash = 0U;
    Board_Leds(0U);
    __set_PRIMASK(saved_primask);
}

/**
 * @brief 依次显示启动灯。
 */
void Indicators_Startup(void)
{
    uint8_t index;
    const uint8_t lamps[] =
    {
        LED_MASK_RED,
        LED_MASK_GREEN,
        LED_MASK_WHITE
    };

    Indicators_Stop();

    for (index = 0U; index < 3U; ++index)
    {
        Board_Leds(lamps[index]);
        Board_DelayMs(200U);
        Board_Leds(0U);
        Board_DelayMs(200U);
    }
}

/**
 * @brief 显示无线初始化完成提示。
 */
void Indicators_Ready(void)
{
    Indicators_Stop();
    Board_Leds(LED_MASK_GREEN);
    Board_DelayMs(500U);
    Board_Leds(0U);
}

/**
 * @brief 进入配对状态并点亮配对指示灯。
 * @param now 当前系统毫秒时间戳。
 */
void Indicators_PairStart(uint32_t now)
{
    uint32_t saved_primask;

    saved_primask = __get_PRIMASK();
    __disable_irq();
    indicator_state.timed = 0U;
    indicator_state.pair_active = 1U;
    indicator_state.pair_flash = 0U;
    indicator_state.pair_started = now;
    Board_Leds(LED_MASK_WHITE);
    __set_PRIMASK(saved_primask);
}

/**
 * @brief 显示配对成功灯效。
 * @param now 当前系统毫秒时间戳。
 */
void Indicators_PairSuccess(uint32_t now)
{
    uint32_t saved_primask;

    saved_primask = __get_PRIMASK();
    __disable_irq();
    indicator_state.timed = 0U;
    indicator_state.pair_active = 1U;
    indicator_state.pair_flash = 1U;
    indicator_state.flash_steps = 0U;
    indicator_state.pair_started = now;
    Board_Leds(0U);
    __set_PRIMASK(saved_primask);
}

/**
 * @brief 显示接收到有效数据的灯效。
 * @param now 当前系统毫秒时间戳。
 */
void Indicators_RxPulse(uint32_t now)
{
    TimedLight(now, 1000UL, LED_MASK_GREEN);
}

/**
 * @brief 将按键位映射为反馈灯掩码。
 * @param key_mask 按键位掩码。
 * @return 对应的灯掩码。
 */
uint8_t Indicators_KeyLedMask(uint8_t key_mask)
{
    if (key_mask & KEY_W1_MASK)
    {
        return LED_MASK_RED;
    }

    if (key_mask & KEY_W2_MASK)
    {
        return LED_MASK_BLUE;
    }

    if (key_mask & KEY_W3_MASK)
    {
        return LED_MASK_GREEN;
    }

    if (key_mask & KEY_W4_MASK)
    {
        return LED_MASK_YELLOW;
    }

    if (key_mask & KEY_W5_MASK)
    {
        return LED_MASK_WHITE;
    }

    return LED_MASK_RED | LED_MASK_GREEN | LED_MASK_WHITE;
}

/**
 * @brief ACK 匹配后点亮对应反馈灯。
 * @param now 当前系统毫秒时间戳。
 * @param key_mask 按键位掩码。
 */
void Indicators_KeyAck(uint32_t now, uint8_t key_mask)
{
    TimedLight(now, ACK_LED_MS, Indicators_KeyLedMask(key_mask));
}

/**
 * @brief 将接收命令映射为反馈灯掩码。
 * @param command 协议命令编号。
 * @return 对应的灯掩码。
 */
uint8_t Indicators_CommandLedMask(uint8_t command)
{
    if (command == 3U)
    {
        return LED_MASK_RED;
    }

    if (command == 4U)
    {
        return LED_MASK_BLUE;
    }

    if (command == 5U)
    {
        return LED_MASK_GREEN;
    }

    if (command == 6U)
    {
        return LED_MASK_YELLOW;
    }

    if (command == 7U)
    {
        return LED_MASK_WHITE;
    }

    return LED_MASK_RED | LED_MASK_GREEN | LED_MASK_WHITE;
}

/**
 * @brief 接收端处理命令时点亮对应反馈灯。
 * @param now 当前系统毫秒时间戳。
 * @param command 协议命令编号。
 */
void Indicators_CommandAck(uint32_t now, uint8_t command)
{
    TimedLight(now, ACK_LED_MS, Indicators_CommandLedMask(command));
}

/**
 * @brief 根据错误码输出故障灯效。
 * @param now 当前系统毫秒时间戳。
 * @param error 无线驱动错误码。
 */
void Indicators_Fault(uint32_t now, uint8_t error)
{
    uint32_t code;
    uint32_t phase;

    code = error;

    if (!code)
    {
        Indicators_Stop();
        return;
    }

    phase = now % (code * 300UL + 1200UL);
    indicator_state.timed = 0U;
    indicator_state.pair_active = 0U;
    indicator_state.pair_flash = 0U;
    Board_Leds((phase < code * 300UL && phase % 300UL < 120UL) ?
               LED_MASK_RED : 0U);
}

/**
 * @brief 在 SysTick 中推进灯效计时。
 * @param now 当前系统毫秒时间戳。
 */
void Indicators_Tick1ms(uint32_t now)
{
    if (indicator_state.timed &&
        (uint32_t)(now - indicator_state.started) >=
            indicator_state.duration)
    {
        indicator_state.timed = 0U;
        Board_Leds(0U);
    }

    if (indicator_state.pair_active)
    {
        if (!indicator_state.pair_flash)
        {
            Board_Leds(LED_MASK_WHITE);
        }
        else if ((uint32_t)(now - indicator_state.pair_started) >= 100UL)
        {
            indicator_state.pair_started += 100UL;
            ++indicator_state.flash_steps;

            if (indicator_state.flash_steps >= 5U)
            {
                indicator_state.pair_flash = 0U;
                Board_Leds(LED_MASK_WHITE);
            }
            else
            {
                Board_Leds((indicator_state.flash_steps == 1U ||
                            indicator_state.flash_steps == 3U) ?
                           LED_MASK_WHITE : 0U);
            }
        }
    }
}
