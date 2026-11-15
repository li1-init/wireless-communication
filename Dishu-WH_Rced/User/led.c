#include "led.h"
#include "board_config.h"
#include "stm32l1xx.h"

void LED_Init(void)
{
    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin = LED_R_CTR | LED_G_CTR;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_40MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpio);

    LED_AllOff();
}

void LED_Set(led_color_t color, led_state_t state)
{
    switch (color)
    {
        case LED_RED:
            if (state == LED_STATE_ON)
            {
                LED_R_ON;
            }
            else
            {
                LED_R_OFF;
            }
            break;

        case LED_GREEN:
            if (state == LED_STATE_ON)
            {
                LED_G_ON;
            }
            else
            {
                LED_G_OFF;
            }
            break;

        default:
            break;
    }
}

void LED_AllOff(void)
{
    LED_R_OFF;
    LED_G_OFF;
}
