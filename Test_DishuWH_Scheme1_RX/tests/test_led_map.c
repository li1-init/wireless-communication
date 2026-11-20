#include <assert.h>
#include <stdio.h>
#include "../User/indicators.h"
#include "../User/key_input.h"

static uint8_t leds;
void Board_Leds(uint8_t mask){leds=mask;}
void Board_DelayMs(uint32_t ms){(void)ms;}

int main(void)
{
    assert(Indicators_KeyLedMask(KEY_W1_MASK)==LED_MASK_RED);
    assert(Indicators_KeyLedMask(KEY_W2_MASK)==LED_MASK_BLUE);
    assert(Indicators_KeyLedMask(KEY_W3_MASK)==LED_MASK_GREEN);
    assert(Indicators_KeyLedMask(KEY_W4_MASK)==LED_MASK_YELLOW);
    assert(Indicators_KeyLedMask(KEY_W5_MASK)==LED_MASK_WHITE);
    assert(Indicators_KeyLedMask(KEY_SW_MASK)==
           (LED_MASK_RED|LED_MASK_GREEN|LED_MASK_WHITE));

    Indicators_KeyAck(0U,KEY_W1_MASK);assert(leds==LED_MASK_RED);
    Indicators_KeyAck(0U,KEY_W2_MASK);assert(leds==LED_MASK_BLUE);
    Indicators_KeyAck(0U,KEY_W3_MASK);assert(leds==LED_MASK_GREEN);
    Indicators_KeyAck(0U,KEY_W4_MASK);assert(leds==LED_MASK_YELLOW);
    Indicators_KeyAck(0U,KEY_W5_MASK);assert(leds==LED_MASK_WHITE);
    Indicators_KeyAck(100U,KEY_W2_MASK);assert(leds==LED_MASK_BLUE);
    Indicators_Tick1ms(199U);assert(leds==LED_MASK_BLUE);
    Indicators_Tick1ms(200U);assert(leds==0U);
    puts("PASS W1-W5 map to red/blue/green/yellow/white ACK LEDs");
    return 0;
}
