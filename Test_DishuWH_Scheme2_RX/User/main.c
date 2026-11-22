/**
 * @file main.c
 * @brief 遥控器应用程序入口。
 */
#include "app_fast.h"
#include "radio_port.h"

/**
 * @brief 初始化应用并周期性执行主循环。
 * @return 按嵌入式应用约定不会正常返回。
 */
int main(void)
{
    AppFast_Init();

    while (1)
    {
        AppFast_Poll();
        Board_DelayMs(1U);
    }
}
