#ifndef APP_RADIO_H
#define APP_RADIO_H

#include <stdint.h>
#include "app_protocol.h"

extern volatile uint8_t fixRadioPacket[APP_RADIO_PACKET_SIZE];

void AppRadio_Init(void);
void AppRadio_StartReceive(void);
void AppRadio_SendPacket(const volatile uint8_t *packet);
void AppRadio_ReadPacket(volatile uint8_t *packet);
void AppRadio_EnterSleep(void);
void AppRadio_HandleIrq(void);
volatile uint8_t *AppRadio_GetReceivedPacket(void);

#endif /* APP_RADIO_H */
