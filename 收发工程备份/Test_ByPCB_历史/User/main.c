/* ============================================================================
 * Test_ByPCB.c
 * Pin map: measured physical-board map after comparing both test builds.
 *
 * Behaviour:
 *   1) Power on : RED, GREEN and WHITE LEDs blink together for 3 complete
 *                 on/off cycles.  The full startup test lasts 3 seconds total.
 *   2) Then     : KEY1 (PA0 / KEY_SW)   toggles RED   LED (PB3)
 *                 KEY2 (PB2 / KEY_USER) toggles GREEN LED (PA11)
 *                 KEY3 (PB1 / KEY_CODE) toggles WHITE LED (PA9)
 *
 * The large W / White LED is LED_W_CTR on PA9.  CODE is KEY_CODE on PB1.
 *
 * Key circuit on the PCB: switch to 3V3 + 2M pull-down (R14/R18/R17)
 *   -> key pressed = pin reads HIGH.
 * ==========================================================================*/
#include "stm32l1xx.h"
#include "board_config.h"
#include "platform.h"
#include "led.h"

#define KEY_DEBOUNCE_MS     20U
#define STARTUP_CYCLES      3U
#define STARTUP_ON_MS       500U
#define STARTUP_OFF_MS      500U

static void KeyPins_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

    gpio.GPIO_Mode  = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_40MHz;
    gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;      /* external 2M pull-down */

    gpio.GPIO_Pin = KEY1_PIN;
    GPIO_Init(KEY1_PORT, &gpio);

    gpio.GPIO_Pin = KEY2_PIN;
    GPIO_Init(KEY2_PORT, &gpio);

    gpio.GPIO_Pin = KEY3_PIN;
    GPIO_Init(KEY3_PORT, &gpio);
}

static uint8_t Key_IsPressed(GPIO_TypeDef* port, uint16_t pin)
{
    if (GPIO_ReadInputDataBit(port, pin) != Bit_SET)
    {
        return 0U;
    }
    Delay_TIM6_1ms(KEY_DEBOUNCE_MS);
    return (GPIO_ReadInputDataBit(port, pin) == Bit_SET) ? 1U : 0U;
}

int main(void)
{
    uint8_t red_on   = 0U;
    uint8_t green_on = 0U;
    uint8_t white_on = 0U;

    RCC_Config();
    TIM6_Delay_Config();

    KeyPins_Init();
    LED_Init();

    /* ---- 3 s total: R/G/W together, 500 ms on + 500 ms off x 3 ---- */
    {
        uint8_t i;
        for (i = 0U; i < STARTUP_CYCLES; ++i)
        {
            LED_Set(LED_RED,   LED_STATE_ON);
            LED_Set(LED_GREEN, LED_STATE_ON);
            LED_Set(LED_WHITE, LED_STATE_ON);
            Delay_TIM6_1ms(STARTUP_ON_MS);

            LED_AllOff();
            Delay_TIM6_1ms(STARTUP_OFF_MS);
        }
        LED_AllOff();
    }

    /* ---- Main loop: each key toggles its own LED ---- */
    while (1)
    {
        if (Key_IsPressed(KEY1_PORT, KEY1_PIN) != 0U)
        {
            red_on = (uint8_t)(!red_on);
            LED_Set(LED_RED, red_on ? LED_STATE_ON : LED_STATE_OFF);
            while (GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == Bit_SET) { }
        }

        if (Key_IsPressed(KEY2_PORT, KEY2_PIN) != 0U)
        {
            green_on = (uint8_t)(!green_on);
            LED_Set(LED_GREEN, green_on ? LED_STATE_ON : LED_STATE_OFF);
            while (GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == Bit_SET) { }
        }

        if (Key_IsPressed(KEY3_PORT, KEY3_PIN) != 0U)
        {
            white_on = (uint8_t)(!white_on);
            LED_Set(LED_WHITE, white_on ? LED_STATE_ON : LED_STATE_OFF);
            while (GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == Bit_SET) { }
        }
    }
}
