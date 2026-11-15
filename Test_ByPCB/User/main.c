/* ============================================================================
 * Test_ByPCB.c
 * Pin map: measured physical-board map after comparing both test builds.
 *
 * Behaviour:
 *   1) Power on : R, G and B LEDs are tested one at a time for 3 rounds.
 *                 This makes a single faulty channel obvious.
 *   2) Then     : KEY1 (PA0 / KEY_SW)   toggles RED   LED (PB3)
 *                 KEY2 (PB2 / KEY_USER) toggles GREEN LED (PA11)
 *                 KEY3 (PB1 / KEY_CODE) toggles BLUE  LED (PB15)
 *
 * Blue is ported from the working Test_BySchematic build: PB15 is driven
 * high, while PA12, PA9, PA8 and PB14 stay low.
 *
 * Key circuit on the PCB: switch to 3V3 + 2M pull-down (R14/R18/R17)
 *   -> key pressed = pin reads HIGH.
 * ==========================================================================*/
#include "stm32l1xx.h"
#include "board_config.h"
#include "platform.h"
#include "led.h"

#define KEY_DEBOUNCE_MS     20U

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
    uint8_t blue_on  = 0U;

    RCC_Config();
    TIM6_Delay_Config();

    KeyPins_Init();
    LED_Init();

    /* ---- Power-on self test: test each channel separately ---- */
    {
        uint8_t i;
        const led_color_t colors[3] = {
            LED_RED, LED_GREEN, LED_BLUE
        };

        for (i = 0U; i < 3U; ++i)
        {
            uint8_t color;
            for (color = 0U; color < 3U; ++color)
            {
                LED_Set(colors[color], LED_STATE_ON);
                Delay_TIM6_1ms(600U);
                LED_AllOff();
                Delay_TIM6_1ms(250U);
            }
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
            blue_on = (uint8_t)(!blue_on);
            LED_Set(LED_BLUE, blue_on ? LED_STATE_ON : LED_STATE_OFF);
            while (GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == Bit_SET) { }
        }
    }
}
