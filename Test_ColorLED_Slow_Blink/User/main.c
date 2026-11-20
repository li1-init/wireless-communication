#include "app_fast.h"
#include "board.h"
int main(void)
{
#if DISHU_COLOR_LED_HIGH_TEST
    (void)Board_Init();
    Board_ColorLedHighTestInit();
    for(;;) {
        Board_ColorLedHighTestSet(1U);
        Board_DelayMs(2000U);
        Board_ColorLedHighTestSet(0U);
        Board_DelayMs(2000U);
    }
#else
    AppFast_Init();
    for(;;){AppFast_Poll();Board_DelayMs(1);}
#endif
}
