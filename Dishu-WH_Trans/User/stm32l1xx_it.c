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
        AppEvent_OnReceiveTimerExpired();
        TIM_Cmd(TIM7, DISABLE);
        TIM_ClearITPendingBit(TIM7, TIM_FLAG_Update);
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetFlagStatus(TIM2, TIM_FLAG_Update) != RESET)
    {
        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
        AppEvent_OnAuxTimerExpired();
        TIM_Cmd(TIM2, DISABLE);
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
    }
}

void EXTI0_IRQHandler(void)
{
    EXTI->IMR &= ~EXTILINE_KEY_SW;
    if (EXTI_GetITStatus(EXTILINE_KEY_SW))
    {
        EXTI_ClearITPendingBit(EXTILINE_KEY_SW);
        AppEvent_OnPowerKeyPressed();
    }
    else
    {
        EXTI->IMR |= EXTILINE_KEY_SW;
    }
}

void EXTI1_IRQHandler(void)
{
    EXTI->IMR &= ~EXTILINE_KEY_AUX;
    if (EXTI_GetITStatus(EXTILINE_KEY_AUX))
    {
        EXTI_ClearITPendingBit(EXTILINE_KEY_AUX);
        AppEvent_OnAuxKeyPressed();
    }
    else
    {
        EXTI->IMR |= EXTILINE_KEY_AUX;
    }
}

void EXTI2_IRQHandler(void)
{
    EXTI->IMR &= ~EXTILINE_KEY_USER;
    if (EXTI_GetITStatus(EXTILINE_KEY_USER))
    {
        EXTI_ClearITPendingBit(EXTILINE_KEY_USER);
        AppEvent_OnUserKeyPressed();
    }
    else
    {
        EXTI->IMR |= EXTILINE_KEY_USER;
    }
}

void EXTI3_IRQHandler(void)
{
    EXTI->IMR &= ~EXTILINE_SI4438_PKGRECV;
    if (EXTI_GetITStatus(EXTILINE_SI4438_PKGRECV))
    {
        EXTI_ClearITPendingBit(EXTILINE_SI4438_PKGRECV);
        AppEvent_OnRadioPacketStart();
    }
    else
    {
        EXTI->IMR |= EXTILINE_SI4438_PKGRECV;
    }
}

void EXTI4_IRQHandler(void)
{
    EXTI->IMR &= ~EXTILINE_SI4438_EXITRX;
    if (EXTI_GetITStatus(EXTILINE_SI4438_EXITRX))
    {
        EXTI_ClearITPendingBit(EXTILINE_SI4438_EXITRX);
        AppRadio_HandleIrq();
    }
    else
    {
        EXTI->IMR |= EXTILINE_SI4438_EXITRX;
    }
}
