#include "stm32l1xx_it.h"
#include "board_config.h"
#include "platform.h"
#include "app_events.h"
#include "app_radio.h"

void TIM7_IRQHandler(void)
{
    if (TIM_GetFlagStatus(TIM7, TIM_FLAG_Update) != RESET)
    {
        TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
        TIM_ClearITPendingBit(TIM7, TIM_FLAG_Update);
        AppEvent_OnTimerExpired();
        TIM_Cmd(TIM7, DISABLE);
    }
}

void EXTI0_IRQHandler(void)
{
    EXTI->IMR &= ~EXTI_SWITCH_KEY;
    if (EXTI_GetITStatus(EXTI_SWITCH_KEY))
    {
        EXTI_ClearITPendingBit(EXTI_SWITCH_KEY);
        AppEvent_OnUserKeyPressed();
    }

    Delay_TIM6_1ms(200U);
    EXTI->IMR |= EXTI_SWITCH_KEY;
}

void EXTI2_IRQHandler(void)
{
    EXTI->IMR &= ~EXTI_SI_GPIO1;
    if (EXTI_GetITStatus(EXTI_SI_GPIO1))
    {
        EXTI_ClearITPendingBit(EXTI_SI_GPIO1);
        AppRadio_HandleIrq();
    }
    else
    {
        EXTI->IMR |= EXTI_SI_GPIO1;
    }
}
