#ifndef APP_EVENTS_H
#define APP_EVENTS_H

void AppEvent_OnPowerKeyPressed(void);
void AppEvent_OnAuxKeyPressed(void);
void AppEvent_OnUserKeyPressed(void);
void AppEvent_OnAuxTimerExpired(void);
void AppEvent_OnReceiveTimerExpired(void);
void AppEvent_OnRadioPacketStart(void);
void AppEvent_OnRadioPacketReceived(void);

#endif /* APP_EVENTS_H */
