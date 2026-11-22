#ifndef LINK_BOARD_H
#define LINK_BOARD_H
#include "stm32l1xx.h"
#include "radio_port.h"
#include "board_profile.h"
#include "sound.h"

#define RADIO_SPI SPI1
#define RADIO_SCLK_PIN GPIO_Pin_5
#define RADIO_MISO_PIN GPIO_Pin_6
#define RADIO_MOSI_PIN GPIO_Pin_7
#define RADIO_SDN_PIN GPIO_Pin_1
#define LED_MASK_RED 1U
#define LED_MASK_GREEN 2U
#define LED_MASK_WHITE 4U

uint8_t Board_Init(void);
void Board_Leds(uint8_t mask);
uint8_t Board_Keys(void);
uint32_t Board_DeviceId(void);
void Board_Sound(uint8_t event,uint8_t code);
#endif
