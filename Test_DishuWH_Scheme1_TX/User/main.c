#include "app_fast.h"
#include "radio_port.h"
int main(void)
{
    AppFast_Init();
    for(;;){AppFast_Poll();Board_DelayMs(1);}
}
