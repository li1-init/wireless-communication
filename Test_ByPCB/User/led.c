#include "led.h"
#include "board_config.h"
#include "platform.h"
#include "stm32l1xx.h"

/* Test build for the five top LEDs R/B/G/Y/W.
 * All channels are active-high push-pull outputs. */
static void LED_OutputInit(void)
{
    GPIO_InitTypeDef gpio;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

    /* PB3 = JTDO / PB4 = JNTRST are JTAG pins after reset.  The PCB uses
       them as LED outputs.  On STM32L1, configuring the pin as a normal
       GPIO (input/output) removes it from the SWJ alternate function;
       SWD on PA13/PA14 keeps working, so re-flashing over ST-Link is OK. */

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

    gpio.GPIO_Pin = LED_W_PIN;
    GPIO_Init(LED_W_PORT, &gpio);

    gpio.GPIO_Pin = LED_W1_PIN;
    GPIO_Init(LED_W1_PORT, &gpio);
    gpio.GPIO_Pin = LED_W2_PIN;
    GPIO_Init(LED_W2_PORT, &gpio);
    gpio.GPIO_Pin = LED_W3_PIN;
    GPIO_Init(LED_W3_PORT, &gpio);
    gpio.GPIO_Pin = LED_W4_PIN;
    GPIO_Init(LED_W4_PORT, &gpio);
    gpio.GPIO_Pin = LED_W5_PIN;
    GPIO_Init(LED_W5_PORT, &gpio);
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
        case LED_WHITE:
            if (on) { GPIO_SetBits(LED_W_PORT, LED_W_PIN); }
            else    { GPIO_ResetBits(LED_W_PORT, LED_W_PIN); }
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
    GPIO_ResetBits(LED_W_PORT, LED_W_PIN);
    GPIO_ResetBits(LED_W1_PORT, LED_W1_PIN);
    GPIO_ResetBits(LED_W2_PORT, LED_W2_PIN);
    GPIO_ResetBits(LED_W3_PORT, LED_W3_PIN);
    GPIO_ResetBits(LED_W4_PORT, LED_W4_PIN);
    GPIO_ResetBits(LED_W5_PORT, LED_W5_PIN);
}

void LED_SetPosition(uint8_t position, led_state_t state)
{
    GPIO_TypeDef *port;
    uint16_t pin;

    switch (position)
    {
        case 1: port = LED_W1_PORT; pin = LED_W1_PIN; break;
        case 2: port = LED_W2_PORT; pin = LED_W2_PIN; break;
        case 3: port = LED_W3_PORT; pin = LED_W3_PIN; break;
        case 4: port = LED_W4_PORT; pin = LED_W4_PIN; break;
        case 5: port = LED_W5_PORT; pin = LED_W5_PIN; break;
        default: return;
    }

    if (state == LED_STATE_ON)
    {
        GPIO_SetBits(port, pin);
    }
    else
    {
        GPIO_ResetBits(port, pin);
    }
}
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

