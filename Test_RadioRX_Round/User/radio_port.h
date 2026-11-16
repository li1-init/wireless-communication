#ifndef LINK_RADIO_PORT_H
#define LINK_RADIO_PORT_H
#include <stdint.h>
uint32_t Board_Millis(void);
void Board_DelayMs(uint32_t ms);
void Board_RadioInit(void);
void Board_RadioSpeed(uint8_t fast);
void Board_Nsel(uint8_t high);
void Board_Sdn(uint8_t high);
uint8_t Board_SpiByte(uint8_t tx,uint8_t *rx);
uint8_t Board_SpiFinish(void);
#endif
