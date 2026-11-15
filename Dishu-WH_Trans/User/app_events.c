#include "app_state.h"
#include "app_events.h"
#include "app_protocol.h"

void AppEvent_OnPowerKeyPressed(void)
{
    switchkey_irq_flag = 1U;
    AppProtocol_SetTxCommand(ALARM_SWITCHOFF);
}

void AppEvent_OnAuxKeyPressed(void)
{
    auxkey_irq_flag = 1U;
    AppProtocol_SetTxCommand(ALARM_TRIG_TB1st);
}

void AppEvent_OnUserKeyPressed(void)
{
    userkey_irq_flag = 1U;
    AppProtocol_SetTxCommand(ALARM_CODING);
}

void AppEvent_OnAuxTimerExpired(void)
{
    tim2_irq_flag = 1U;
}

void AppEvent_OnReceiveTimerExpired(void)
{
    tim7_irq_flag = 1U;
}

void AppEvent_OnRadioPacketStart(void)
{
    si4438pkgrecv_irq_flag = 1U;
}

void AppEvent_OnRadioPacketReceived(void)
{
    si4438_validpkg_flag = 1U;
}
