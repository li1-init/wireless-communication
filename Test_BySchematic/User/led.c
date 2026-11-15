#include "led.h"
#include "board_config.h"
#include "platform.h"
#include "stm32l1xx.h"

/* Test build: LED pins come straight from board_config.h.
 * GPIO high = LED on (N-MOS low-side switch, see PCB AO3400A). */
static void LED_OutputInit(void)
{
    GPIO_InitTypeDef gpio;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

    gpio.GPIO_Mode  = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_40MHz;
    gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    gpio.GPIO_Pin = LED_R_PIN;
    GPIO_Init(LED_R_PORT, &gpio);

    gpio.GPIO_Pin = LED_G_PIN;
    GPIO_Init(LED_G_PORT, &gpio);

    gpio.GPIO_Pin = LED_B_PIN;
    GPIO_Init(LED_B_PORT, &gpio);

    gpio.GPIO_Pin = LED_Y_PIN;
    GPIO_Init(LED_Y_PORT, &gpio);
}

void LED_Init(void)
{
    LED_OutputInit();
    LED_AllOff();
}

void LED_Set(led_color_t color, led_state_t state)
{
    uint8_t on = (state == LED_STATE_ON);

    switch (color)
    {
        case LED_RED:
            if (on) { GPIO_SetBits(LED_R_PORT, LED_R_PIN); }
            else    { GPIO_ResetBits(LED_R_PORT, LED_R_PIN); }
            break;
        case LED_GREEN:
            if (on) { GPIO_SetBits(LED_G_PORT, LED_G_PIN); }
            else    { GPIO_ResetBits(LED_G_PORT, LED_G_PIN); }
            break;
        case LED_BLUE:
            if (on) { GPIO_SetBits(LED_B_PORT, LED_B_PIN); }
            else    { GPIO_ResetBits(LED_B_PORT, LED_B_PIN); }
            break;
        case LED_YELLOW:
            if (on) { GPIO_SetBits(LED_Y_PORT, LED_Y_PIN); }
            else    { GPIO_ResetBits(LED_Y_PORT, LED_Y_PIN); }
            break;
        default:
            break;
    }
}

void LED_AllOff(void)
{
    GPIO_ResetBits(LED_R_PORT, LED_R_PIN);
    GPIO_ResetBits(LED_G_PORT, LED_G_PIN);
    GPIO_ResetBits(LED_B_PORT, LED_B_PIN);
    GPIO_ResetBits(LED_Y_PORT, LED_Y_PIN);
}

void LED_SetPosition(uint8_t position, led_state_t state) { (void)position; (void)state; }
void CodeLED_Display(uint8_t led_code, uint8_t state)     { (void)led_code; (void)state; }
void WhiteLED_Display(uint8_t led_position, uint8_t state){ (void)led_position; (void)state; }

void LED_Blink(led_color_t color, uint8_t count, uint16_t interval_ms)
{
    uint8_t i;

    for (i = 0U; i < count; ++i)
    {
        LED_Set(color, LED_STATE_ON);
        Delay_TIM6_1ms(interval_ms);
        LED_Set(color, LED_STATE_OFF);
        Delay_TIM6_1ms(interval_ms);
    }
}
