/* ============================================================================
 * Test_ByPCB.c
 * Quick test for the first two small 3 mm white LEDs:
 *
 *   W1 D2  PB9  / pin 46  / upper pad +, lower pad -
 *   W2 D4  PB6  / pin 42  / upper pad +, lower pad -
 *
 * W1 and W2 light together for 3 s, then all outputs turn off for 1 s.
 * The pattern repeats so polarity and continuity can be checked easily.
 * ==========================================================================*/
#include "stm32l1xx.h"
#include "board_config.h"
#include "platform.h"
#include "led.h"

int main(void)
{
    RCC_Config();
    TIM6_Delay_Config();
    LED_Init();

    while (1)
    {
        LED_AllOff();
        LED_SetPosition(1U, LED_STATE_ON);
        LED_SetPosition(2U, LED_STATE_ON);
        Delay_TIM6_1ms(3000U);
        LED_AllOff();
        Delay_TIM6_1ms(1000U);
    }
}
