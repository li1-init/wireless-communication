#include <stdio.h>
#include <stdlib.h>
#include "../User/sound.h"
static unsigned on,elapsed,edges,total_on,cases;
#define REQUIRE(x) do { if(!(x)) { fprintf(stderr,"FAIL sound line %d: %s at %u ms\n",__LINE__,#x,elapsed);exit(2); } } while(0)
void Board_SoundGate(uint8_t value)
{
    if(on!=value) ++edges;
    on=value;
}
static void Tick(unsigned n)
{
    while(n--) {
        if(on) ++total_on;
        ++elapsed;
        Sound_Tick1ms();
    }
}
static void Start(uint8_t event,uint8_t code)
{
    Sound_Play(0,0);on=elapsed=edges=total_on=0;
    Sound_Play(event,code);
}
int main(void)
{
    unsigned i;
    Start(SOUND_BOOT,0);
    REQUIRE(on);Tick(349);REQUIRE(on);Tick(1);REQUIRE(!on);
    Tick(1000);REQUIRE(total_on==350&&edges==2);++cases;
    puts("PASS boot: exactly one 350ms tone");
    Start(SOUND_PACKET,0);
    Tick(119);REQUIRE(on);Tick(1);REQUIRE(!on);
    REQUIRE(total_on==120&&edges==2);++cases;
    puts("PASS received packet: one 120ms tone");
    for(i=1;i<=9;++i) {
        Start(SOUND_FAULT,(uint8_t)i);
        Tick(500);REQUIRE(!on);Tick(250);REQUIRE(on);
        Tick(i*120+(i-1)*180);
        REQUIRE(!on&&total_on==500+i*120&&edges==2+2*i);
    }
    ++cases;puts("PASS fault codes 1..9: long prefix and exact short-tone count");
    Start(SOUND_PACKET,0);Tick(10);Sound_Play(SOUND_FAULT,2);
    Tick(2000);REQUIRE(!on);++cases;
    puts("PASS fault replaces an in-progress receive cue");
    Start(SOUND_PACKET,0);
    /* Foreground/driver never runs here: SysTick alone must silence buzzer. */
    Tick(1000);REQUIRE(!on&&total_on==120);++cases;
    puts("PASS no sustained tone during a blocked foreground SPI wait");
    Start(0,0);Tick(5000);REQUIRE(!on&&edges==0);++cases;
    puts("PASS idle/no packet remains silent");
    printf("%u sound cases passed; 0 failures\n",cases);
    return 0;
}
