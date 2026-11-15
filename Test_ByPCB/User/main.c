/* ============================================================================
 * Test_ByPCB.c
 * Top five colour LEDs: R -> B -> G -> Y -> W, left to right on the PCB.
 *
 * Net mapping from "发射.pcb":
 *   RED    PB3  / pin 39
 *   BLUE   PA12 / pin 33
 *   GREEN  PA11 / pin 32
 *   YELLOW PA10 / pin 31
 *   WHITE  PA9  / pin 30
 *
 * Each LED lights alone for 1.5 s, then all five light together for 1.2 s.
 * All LED channels are active high.
 * ==========================================================================*/
#include "stm32l1xx.h"
#include "board_config.h"
#include "platform.h"
#include "led.h"

int main(void)
{
    const led_color_t colors[5] = {
        LED_RED, LED_BLUE, LED_GREEN, LED_YELLOW, LED_WHITE
    };

    RCC_Config();
    TIM6_Delay_Config();
    LED_Init();

    while (1)
    {
        uint8_t color;

        /* One channel at a time: R, B, G, Y, W. */
        for (color = 0U; color < 5U; ++color)
        {
            LED_AllOff();
            LED_Set(colors[color], LED_STATE_ON);
            Delay_TIM6_1ms(1500U);
        }

        /* All five together makes missing or reversed LEDs easy to spot. */
        LED_AllOff();
        Delay_TIM6_1ms(500U);
        for (color = 0U; color < 5U; ++color)
        {
            LED_Set(colors[color], LED_STATE_ON);
        }
        Delay_TIM6_1ms(1200U);

        LED_AllOff();
        Delay_TIM6_1ms(1000U);
    }
}
