/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：方形接收板灯控模块，用非阻塞计时实现收包和对码灯效。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 引入本模块所需的头文件接口。 */
#include "indicators.h"
/* 引入本模块所需的头文件接口。 */
#include "board.h"
/* 引入本模块所需的头文件接口。 */
#include "key_input.h"
/* 执行当前语句。 */
static volatile uint8_t timed,pair_active,pair_flash,flash_steps;
/* 执行当前语句。 */
static volatile uint32_t started,duration,pair_started;
/** @brief 在临界区内设置 LED 及自动熄灭计时。 */
/* 进入本模块的函数实现。 */
static void TimedLight(uint32_t now,uint32_t ms,uint8_t mask)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t saved=__get_PRIMASK();
/* 调用相关模块接口完成当前处理步骤。 */
    __disable_irq();
/* 更新运行状态或计算结果。 */
    timed=1U; started=now; duration=ms;
/* 更新运行状态或计算结果。 */
    pair_active=0U; pair_flash=0U;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Leds(mask);
/* 调用相关模块接口完成当前处理步骤。 */
    __set_PRIMASK(saved);
/* 结束当前代码块。 */
}
/** @brief 取消全部灯控状态并关闭 LED。 */
/* 进入本模块的函数实现。 */
void Indicators_Stop(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t saved=__get_PRIMASK();
/* 调用相关模块接口完成当前处理步骤。 */
    __disable_irq();
/* 更新运行状态或计算结果。 */
    timed=0U;
/* 更新运行状态或计算结果。 */
    pair_active=0U;
/* 更新运行状态或计算结果。 */
    pair_flash=0U;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Leds(0U);
/* 调用相关模块接口完成当前处理步骤。 */
    __set_PRIMASK(saved);
/* 结束当前代码块。 */
}
/** @brief 依次显示启动灯，时序与原方板发送工程一致。 */
/* 进入本模块的函数实现。 */
void Indicators_Startup(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t i;
/* 定义或初始化本模块使用的状态数据。 */
    const uint8_t lamps[]={LED_MASK_RED,LED_MASK_GREEN,LED_MASK_WHITE};
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Stop();
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<3U;++i) {
/* 调用相关模块接口完成当前处理步骤。 */
        Board_Leds(lamps[i]); Board_DelayMs(200U);
/* 调用相关模块接口完成当前处理步骤。 */
        Board_Leds(0U); Board_DelayMs(200U);
/* 结束当前代码块。 */
    }
/* 结束当前代码块。 */
}
/** @brief 显示本地无线初始化完成的绿灯提示。 */
/* 进入本模块的函数实现。 */
void Indicators_Ready(void)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Stop();
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Leds(LED_MASK_GREEN); Board_DelayMs(500U); Board_Leds(0U);
/* 结束当前代码块。 */
}
/** @brief 进入对码状态，白灯常亮。 */
/* 进入本模块的函数实现。 */
void Indicators_PairStart(uint32_t now)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t saved=__get_PRIMASK();
/* 调用相关模块接口完成当前处理步骤。 */
    __disable_irq();
/* 更新运行状态或计算结果。 */
    timed=0U;
/* 更新运行状态或计算结果。 */
    pair_active=1U;
/* 更新运行状态或计算结果。 */
    pair_flash=0U;
/* 更新运行状态或计算结果。 */
    pair_started=now;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Leds(LED_MASK_WHITE);
/* 调用相关模块接口完成当前处理步骤。 */
    __set_PRIMASK(saved);
/* 结束当前代码块。 */
}
/** @brief 对码成功后白灯快速闪烁两次，然后恢复常亮。 */
/* 进入本模块的函数实现。 */
void Indicators_PairSuccess(uint32_t now)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t saved=__get_PRIMASK();
/* 调用相关模块接口完成当前处理步骤。 */
    __disable_irq();
/* 更新运行状态或计算结果。 */
    timed=0U;
/* 更新运行状态或计算结果。 */
    pair_active=1U;
/* 更新运行状态或计算结果。 */
    pair_flash=1U;
/* 更新运行状态或计算结果。 */
    flash_steps=0U;
/* 更新运行状态或计算结果。 */
    pair_started=now;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Leds(0U);
/* 调用相关模块接口完成当前处理步骤。 */
    __set_PRIMASK(saved);
/* 结束当前代码块。 */
}
/** @brief 收到有效数据后绿灯亮 1 秒。 */
/* 进入本模块的函数实现。 */
void Indicators_RxPulse(uint32_t now)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    TimedLight(now,1000UL,LED_MASK_GREEN);
/* 结束当前代码块。 */
}
/** @brief 将 W1-W5 一一映射到红、蓝、绿、黄、白反馈灯。 */
/* 进入本模块的函数实现。 */
uint8_t Indicators_KeyLedMask(uint8_t key_mask)
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(key_mask&KEY_W1_MASK)return LED_MASK_RED;
/* 根据运行条件选择处理分支。 */
    if(key_mask&KEY_W2_MASK)return LED_MASK_BLUE;
/* 根据运行条件选择处理分支。 */
    if(key_mask&KEY_W3_MASK)return LED_MASK_GREEN;
/* 根据运行条件选择处理分支。 */
    if(key_mask&KEY_W4_MASK)return LED_MASK_YELLOW;
/* 根据运行条件选择处理分支。 */
    if(key_mask&KEY_W5_MASK)return LED_MASK_WHITE;
/* 返回当前函数的处理结果。 */
    return LED_MASK_RED|LED_MASK_GREEN|LED_MASK_WHITE;
/* 结束当前代码块。 */
}
/** @brief ACK 匹配后仅点亮本次按键对应的灯 100 毫秒。 */
/* 进入本模块的函数实现。 */
void Indicators_KeyAck(uint32_t now,uint8_t key_mask)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    TimedLight(now,ACK_LED_MS,Indicators_KeyLedMask(key_mask));
/* 结束当前代码块。 */
}
/** @brief 将协议命令 3～7 一一映射到红、蓝、绿、黄、白反馈灯。 */
/* 进入本模块的函数实现。 */
uint8_t Indicators_CommandLedMask(uint8_t command)
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(command==3U)return LED_MASK_RED;
/* 根据运行条件选择处理分支。 */
    if(command==4U)return LED_MASK_BLUE;
/* 根据运行条件选择处理分支。 */
    if(command==5U)return LED_MASK_GREEN;
/* 根据运行条件选择处理分支。 */
    if(command==6U)return LED_MASK_YELLOW;
/* 根据运行条件选择处理分支。 */
    if(command==7U)return LED_MASK_WHITE;
/* 返回当前函数的处理结果。 */
    return LED_MASK_RED|LED_MASK_GREEN|LED_MASK_WHITE;
/* 结束当前代码块。 */
}
/** @brief 接收端收到 W1～W5 数据包时点亮与发送端相同的对应灯。 */
/* 进入本模块的函数实现。 */
void Indicators_CommandAck(uint32_t now,uint8_t command)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    TimedLight(now,ACK_LED_MS,Indicators_CommandLedMask(command));
/* 结束当前代码块。 */
}
/** @brief 根据错误码输出循环红灯闪烁。 */
/* 进入本模块的函数实现。 */
void Indicators_Fault(uint32_t now,uint8_t error)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t code=error;
/* 进入本模块的函数实现。 */
    if(!code) { Indicators_Stop(); return; }
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t phase=now%(code*300UL+1200UL);
/* 更新运行状态或计算结果。 */
    timed=0U;
/* 更新运行状态或计算结果。 */
    pair_active=0U;
/* 更新运行状态或计算结果。 */
    pair_flash=0U;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_Leds((phase<code*300UL && phase%300UL<120UL)?LED_MASK_RED:0U);
/* 结束当前代码块。 */
}
/** @brief 在 SysTick 中按时熄灯，并维持对码常亮或成功快闪。 */
/* 进入本模块的函数实现。 */
void Indicators_Tick1ms(uint32_t now)
/* 开始当前代码块。 */
{
/* 进入本模块的函数实现。 */
    if(timed && (uint32_t)(now-started)>=duration) {
/* 更新运行状态或计算结果。 */
        timed=0U;
/* 调用相关模块接口完成当前处理步骤。 */
        Board_Leds(0U);
/* 结束当前代码块。 */
    }
/* 进入本模块的函数实现。 */
    if(pair_active) {
/* 根据运行条件选择处理分支。 */
        if(!pair_flash) Board_Leds(LED_MASK_WHITE);
/* 进入本模块的函数实现。 */
        else if((uint32_t)(now-pair_started)>=100UL) {
/* 执行当前语句。 */
            pair_started+=100UL;
/* 更新运行状态或计算结果。 */
            ++flash_steps;
/* 进入本模块的函数实现。 */
            if(flash_steps>=5U) {
/* 更新运行状态或计算结果。 */
                pair_flash=0U;
/* 调用相关模块接口完成当前处理步骤。 */
                Board_Leds(LED_MASK_WHITE);
/* 说明本行代码在当前模块中的处理作用。 */
            } else Board_Leds((flash_steps==1U || flash_steps==3U)?
/* 执行当前语句。 */
                              LED_MASK_WHITE:0U);
/* 结束当前代码块。 */
        }
/* 结束当前代码块。 */
    }
/* 结束当前代码块。 */
}
