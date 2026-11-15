/* ============================================================================
 * Test_ByPCB.c
 * Five small 3 mm white LED test, physical order left -> right:
 *
 *   W1 D2  PB9  / pin 46  / upper pad +, lower pad -
 *   W2 D4  PB6  / pin 42  / upper pad +, lower pad -
 *   W3 D6  PB8  / pin 45  / upper pad +, lower pad -
 *   W4 D8  PB5  / pin 41  / upper pad +, lower pad -
 *   W5 D11 PB4  / pin 40  / upper pad +, lower pad -
 *
 * Test sequence:
 *   1. W1..W5 light one at a time for 2 s each.
 *   2. All five light together for 1.5 s.
 *   3. W1 and W2 light together for 3 s.
 *   4. Everything turns off, then the full sequence repeats.
 * ==========================================================================*/
#include "stm32l1xx.h"
#include "board_config.h"
#include "platform.h"
#include "led.h"

int main(void)
{
    uint8_t position;

    RCC_Config();
    TIM6_Delay_Config();
    LED_Init();

    while (1)
    {
        /* Existing test: positions 1..5 from left to right. */
        for (position = 1U; position <= 5U; ++position)
        {
            LED_AllOff();
            LED_SetPosition(position, LED_STATE_ON);
            Delay_TIM6_1ms(2000U);
        }

        /* Existing test: all five together. */
        LED_AllOff();
        Delay_TIM6_1ms(500U);
        for (position = 1U; position <= 5U; ++position)
        {
            LED_SetPosition(position, LED_STATE_ON);
        }
        Delay_TIM6_1ms(1500U);

        /* New test: the first two LEDs together, then off. */
        LED_AllOff();
        Delay_TIM6_1ms(500U);
        LED_SetPosition(1U, LED_STATE_ON);
        LED_SetPosition(2U, LED_STATE_ON);
        Delay_TIM6_1ms(3000U);
        LED_AllOff();
        Delay_TIM6_1ms(1000U);
    }
}
