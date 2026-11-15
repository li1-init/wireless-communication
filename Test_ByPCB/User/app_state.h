#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>
#include "stm32l1xx.h"
#include "app_protocol.h"

/* Current command packet. */
extern volatile uint8_t Trans_Send[APP_RADIO_PACKET_SIZE];

/* Button and radio event flags. */
extern volatile uint8_t switchkey_irq_flag;
extern volatile uint8_t userkey_irq_flag;
extern volatile uint8_t auxkey_irq_flag;
extern volatile uint8_t si4438pkgrecv_irq_flag;
extern volatile uint8_t si4438_validpkg_flag;
extern volatile uint8_t tim2_irq_flag;
extern volatile uint8_t tim7_irq_flag;

/* Platform state still shared with the low-power driver. */
extern volatile uint8_t lum_mode;
extern volatile uint8_t power_off_flag;

void AppState_Init(void);
uint8_t AppState_HasButtonEvent(void);
uint8_t AppState_IsRadioPacketReady(void);
void AppState_ClearRadioPacketReady(void);

#endif /* APP_STATE_H */
