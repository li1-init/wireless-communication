#ifndef LED_H
#define LED_H

#include <stdint.h>

typedef enum
{
    LED_RED = 0,
    LED_GREEN
} led_color_t;

typedef enum
{
    LED_STATE_OFF = 0,
    LED_STATE_ON
} led_state_t;

void LED_Init(void);
void LED_Set(led_color_t color, led_state_t state);
void LED_AllOff(void);

#endif /* LED_H */
