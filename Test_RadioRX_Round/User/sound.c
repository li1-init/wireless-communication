#include "sound.h"
static volatile uint16_t durations[20];
static volatile uint16_t remaining;
static volatile uint8_t index_now,count;

void Sound_Play(uint8_t event,uint8_t code)
{
    uint8_t i;
    count=0;remaining=0;index_now=0;
    Board_SoundGate(0);
    if(event==SOUND_BOOT) { durations[0]=350;count=1; }
    else if(event==SOUND_PACKET) {
        durations[0]=120;count=1;
    } else if(event==SOUND_PAIR_ENTER) {
        durations[0]=2000;count=1;
    } else if(event==SOUND_PAIR_OK) {
        durations[0]=500;count=1;
    } else if(event==SOUND_PAIR_EXIT) {
        durations[0]=120;durations[1]=100;durations[2]=120;count=3;
    } else if(event==SOUND_FAULT) {
        if(code<1U||code>9U) code=4U;
        durations[0]=500;durations[1]=250;
        for(i=0;i<code;++i) {
            durations[2U+i*2U]=120;
            durations[3U+i*2U]=180;
        }
        count=(uint8_t)(2U*code+1U);
    }
    if(count) { remaining=durations[0];Board_SoundGate(1); }
}
void Sound_Tick1ms(void)
{
    if(!remaining) return;
    if(--remaining) return;
    ++index_now;
    if(index_now>=count) { Board_SoundGate(0);count=0;return; }
    remaining=durations[index_now];
    Board_SoundGate((index_now&1U)?0U:1U);
}
