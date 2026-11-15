#include "alarm.h"
#include "app_protocol.h"
#include "app_state.h"
#include "buzzer.h"
#include "led.h"
#include "platform.h"

void Alarm_Show(uint8_t command)
{
    uint8_t i;
    const uint32_t hmi_intervel = 100U;

    if (command == ALARM_CODING)
    {
        for (i = 0U; i < 3U; ++i)
        {
            LED_Set(LED_RED, LED_STATE_ON);
            Buzzer_On();
            Delay_TIM6_1ms(hmi_intervel);
            LED_Set(LED_RED, LED_STATE_OFF);
            Buzzer_Off();
            Delay_TIM6_1ms(hmi_intervel);
        }
    }
    else if (command == ALARM_SWITCHOFF)
    {
        for (i = 0U; i < 3U; ++i)
        {
            LED_Set(LED_GREEN, LED_STATE_ON);
            Buzzer_On();
            Delay_TIM6_1ms(hmi_intervel);
            LED_Set(LED_GREEN, LED_STATE_OFF);
            Buzzer_Off();
            Delay_TIM6_1ms(hmi_intervel);
        }
    }
    else if (command == ALARM_TRIG_TB1st)
    {
        for (i = 0U; i < 3U; ++i)
        {
            LED_Set(LED_RED, LED_STATE_ON);
            LED_Set(LED_GREEN, LED_STATE_ON);
            Buzzer_On();
            Delay_TIM6_1ms(hmi_intervel);
            LED_Set(LED_RED, LED_STATE_OFF);
            LED_Set(LED_GREEN, LED_STATE_OFF);
            Buzzer_Off();
            Delay_TIM6_1ms(hmi_intervel);
        }
    }
}
