#ifndef LED_H
#define LED_H

#include <stdint.h>

typedef enum
{
    LED_RED = 0,
    LED_GREEN,
    LED_BLUE,
    LED_YELLOW,
    LED_WHITE
} led_color_t;

typedef enum
{
    LED_STATE_OFF = 0,
    LED_STATE_ON
} led_state_t;

void LED_Init(void);
void LED_Set(led_color_t color, led_state_t state);
void LED_SetPosition(uint8_t position, led_state_t state);
void LED_AllOff(void);
void CodeLED_Display(uint8_t led_code, uint8_t state);
void WhiteLED_Display(uint8_t led_position, uint8_t state);
void LED_Blink(led_color_t color, uint8_t count, uint16_t interval_ms);

#endif /* LED_H */
