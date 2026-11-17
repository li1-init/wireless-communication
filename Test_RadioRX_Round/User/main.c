/* 文件说明：程序入口：初始化应用，然后循环调用业务处理函数。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This file contains only the receiver entry point and scheduler loop. */
#include "app_rx.h"
#include "radio_port.h"
/** @brief 程序入口：初始化后持续轮询应用，循环末尾等待 1 ms。 */
int main(void)
{
    AppRx_Init();
    for(;;) {
        AppRx_Poll();
        Board_DelayMs(1U);
    }
}
