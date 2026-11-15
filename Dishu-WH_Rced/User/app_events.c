#include "app_state.h"
#include "app_events.h"
#include "led.h"

void AppEvent_OnUserKeyPressed(void)
{
    userkey_irq_flag = 1U;
    if (AppState_IsPairingMode() != 0U)
    {
        AppState_SetPairingMode(0U);
        LED_Set(LED_RED, LED_STATE_OFF);
    }
    else
    {
        AppState_SetPairingMode(1U);
        LED_Set(LED_RED, LED_STATE_ON);
    }
}

void AppEvent_OnRadioPacketReceived(void)
{
    si4438_irq_flag = 1U;
}

void AppEvent_OnTimerExpired(void)
{
    tim7_irq_flag = 1U;
}
