#ifndef LINK_SOUND_H
#define LINK_SOUND_H
#include <stdint.h>
enum { SOUND_BOOT=1, SOUND_PACKET=2, SOUND_FAULT=3 };
/* Board_Sound wraps Sound_Play in an interrupt-safe critical section.
 * Sound_Tick1ms runs from SysTick so a stalled SPI command cannot sustain a tone. */
void Sound_Play(uint8_t event,uint8_t code);
void Sound_Tick1ms(void);
void Board_SoundGate(uint8_t on);
#endif
