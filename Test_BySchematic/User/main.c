/* ============================================================================
 * Test_BySchematic.c
 * Pin map: eTrap-01MS Luminator Ver10.pdf  (page 3 / page 4)
 *
 * Behaviour:
 *   1) Power on : all 4 colour LEDs blink together 3 times (300 ms on/off),
 *                 then all LEDs are switched OFF.
 *   2) Then     : KEY1 (PA0) toggles RED   LED on/off
 *                 KEY2 (PB1) toggles GREEN LED on/off
 *                 KEY3 (PB2) toggles BLUE  LED on/off
 *
 * Key circuit on the PDF board: switch to 3V3, 2M pull-down to GND
 *   -> key pressed = pin reads HIGH.
 *
 * NOTE: This build is for the PDF schematic.  If you run it on the real
 *       "发射" PCB, the LED pins are DIFFERENT - use Test_ByPCB instead.
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
    /* pressed = HIGH (switch to 3V3), debounce by re-reading */
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

    /* ---- Power-on self test: blink all LEDs 3 times, then off ---- */
    {
        uint8_t i;
        for (i = 0U; i < 3U; ++i)
        {
            LED_Set(LED_RED,    LED_STATE_ON);
            LED_Set(LED_GREEN,  LED_STATE_ON);
            LED_Set(LED_BLUE,   LED_STATE_ON);
            LED_Set(LED_YELLOW, LED_STATE_ON);
            Delay_TIM6_1ms(300U);

            LED_AllOff();
            Delay_TIM6_1ms(300U);
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
