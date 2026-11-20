#ifndef TEST_BOARD_STUB_H
#define TEST_BOARD_STUB_H
#include <stdint.h>
#define LINK_BOARD_H
#define LED_MASK_RED 1U
#define LED_MASK_GREEN 2U
#define LED_MASK_WHITE 4U
#define LED_MASK_BLUE 8U
#define LED_MASK_YELLOW 16U
static inline uint32_t __get_PRIMASK(void){return 0;}
static inline void __disable_irq(void){}
static inline void __set_PRIMASK(uint32_t v){(void)v;}
void Board_Leds(uint8_t mask);
void Board_DelayMs(uint32_t ms);
uint8_t Board_Init(void);
uint32_t Board_Millis(void);
uint32_t Board_DeviceId(void);
uint8_t Board_Keys(void);
#endif
