#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>
#include "stm32l1xx.h"
#include "app_protocol.h"

/* Runtime event flags shared between interrupts and the application loop. */
extern volatile uint8_t userkey_irq_flag;
extern volatile uint8_t si4438_irq_flag;
extern volatile uint8_t tim7_irq_flag;
extern volatile uint8_t power_off_flag;
extern volatile uint8_t low_valtage_flag;
extern volatile uint8_t pairing_mode_flag;

/* Reusable radio transmit buffer. */
extern volatile uint8_t si4438_txdata[APP_RADIO_PACKET_SIZE];

void AppState_Init(void);
uint8_t AppState_IsRadioPacketReady(void);
uint8_t AppState_IsPairingMode(void);
void AppState_SetPairingMode(uint8_t enabled);

#endif /* APP_STATE_H */
