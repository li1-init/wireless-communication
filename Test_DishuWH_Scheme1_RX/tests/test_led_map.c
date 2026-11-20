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
    assert(Indicators_CommandLedMask(3U)==LED_MASK_RED);
    assert(Indicators_CommandLedMask(4U)==LED_MASK_BLUE);
    assert(Indicators_CommandLedMask(5U)==LED_MASK_GREEN);
    assert(Indicators_CommandLedMask(6U)==LED_MASK_YELLOW);
    assert(Indicators_CommandLedMask(7U)==LED_MASK_WHITE);
    assert(DISHU_KEY_LED_MS==2000UL);

    Indicators_KeyAck(0U,KEY_W1_MASK);assert(leds==LED_MASK_RED);
    Indicators_KeyAck(0U,KEY_W2_MASK);assert(leds==LED_MASK_BLUE);
    Indicators_KeyAck(0U,KEY_W3_MASK);assert(leds==LED_MASK_GREEN);
    Indicators_KeyAck(0U,KEY_W4_MASK);assert(leds==LED_MASK_YELLOW);
    Indicators_KeyAck(0U,KEY_W5_MASK);assert(leds==LED_MASK_WHITE);
    Indicators_KeyAck(100U,KEY_W2_MASK);assert(leds==LED_MASK_BLUE);
    Indicators_Tick1ms(100U+DISHU_KEY_LED_MS-1U);assert(leds==LED_MASK_BLUE);
    Indicators_Tick1ms(100U+DISHU_KEY_LED_MS);assert(leds==0U);
    Indicators_CommandAck(300U,5U);assert(leds==LED_MASK_GREEN);
    Indicators_Tick1ms(300U+DISHU_KEY_LED_MS-1U);assert(leds==LED_MASK_GREEN);
    Indicators_Tick1ms(300U+DISHU_KEY_LED_MS);assert(leds==0U);
    Indicators_KeyAck(5000U,KEY_W1_MASK);assert(leds==LED_MASK_RED);
    Indicators_KeyAck(6000U,KEY_W2_MASK);assert(leds==LED_MASK_BLUE);
    Indicators_Tick1ms(6999U);assert(leds==LED_MASK_BLUE);
    Indicators_Tick1ms(7999U);assert(leds==LED_MASK_BLUE);
    Indicators_Tick1ms(8000U);assert(leds==0U);
    puts("PASS W1-W5 map to red/blue/green/yellow/white ACK LEDs");
    return 0;
}
