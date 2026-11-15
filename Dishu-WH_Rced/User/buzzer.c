#include "buzzer.h"
#include "board_config.h"
#include "platform.h"
#include "stm32l1xx.h"

void Buzzer_Init(void)
{
    TIM4_Beep_PWMConfig();
    Buzzer_Off();
}

void Buzzer_On(void)
{
    BP_ON;
}

void Buzzer_Off(void)
{
    BP_OFF;
}
