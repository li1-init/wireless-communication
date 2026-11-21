/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 引入本模块所需的头文件接口。 */
#include <assert.h>
/* 引入本模块所需的头文件接口。 */
#include <stdio.h>
/* 引入本模块所需的头文件接口。 */
#include "../User/indicators.h"
/* 引入本模块所需的头文件接口。 */
#include "../User/key_input.h"

/* 定义或初始化本模块使用的状态数据。 */
static uint8_t leds;
/* 进入本模块的函数实现。 */
void Board_Leds(uint8_t mask){leds=mask;}
/* 进入本模块的函数实现。 */
void Board_DelayMs(uint32_t ms){(void)ms;}

/* 进入本模块的函数实现。 */
int main(void)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_KeyLedMask(KEY_W1_MASK)==LED_MASK_RED);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_KeyLedMask(KEY_W2_MASK)==LED_MASK_BLUE);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_KeyLedMask(KEY_W3_MASK)==LED_MASK_GREEN);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_KeyLedMask(KEY_W4_MASK)==LED_MASK_YELLOW);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_KeyLedMask(KEY_W5_MASK)==LED_MASK_WHITE);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_KeyLedMask(KEY_SW_MASK)==
/* 执行当前语句。 */
           (LED_MASK_RED|LED_MASK_GREEN|LED_MASK_WHITE));
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_CommandLedMask(3U)==LED_MASK_RED);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_CommandLedMask(4U)==LED_MASK_BLUE);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_CommandLedMask(5U)==LED_MASK_GREEN);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_CommandLedMask(6U)==LED_MASK_YELLOW);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(Indicators_CommandLedMask(7U)==LED_MASK_WHITE);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(DISHU_KEY_LED_MS==100UL);

/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_KeyAck(0U,KEY_W1_MASK);assert(leds==LED_MASK_RED);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_KeyAck(0U,KEY_W2_MASK);assert(leds==LED_MASK_BLUE);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_KeyAck(0U,KEY_W3_MASK);assert(leds==LED_MASK_GREEN);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_KeyAck(0U,KEY_W4_MASK);assert(leds==LED_MASK_YELLOW);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_KeyAck(0U,KEY_W5_MASK);assert(leds==LED_MASK_WHITE);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_KeyAck(100U,KEY_W2_MASK);assert(leds==LED_MASK_BLUE);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Tick1ms(100U+DISHU_KEY_LED_MS-1U);assert(leds==LED_MASK_BLUE);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Tick1ms(100U+DISHU_KEY_LED_MS);assert(leds==0U);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_CommandAck(300U,5U);assert(leds==LED_MASK_GREEN);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Tick1ms(300U+DISHU_KEY_LED_MS-1U);assert(leds==LED_MASK_GREEN);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Tick1ms(300U+DISHU_KEY_LED_MS);assert(leds==0U);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_KeyAck(5000U,KEY_W1_MASK);assert(leds==LED_MASK_RED);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_KeyAck(6000U,KEY_W2_MASK);assert(leds==LED_MASK_BLUE);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Tick1ms(6099U);assert(leds==LED_MASK_BLUE);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Tick1ms(6099U);assert(leds==LED_MASK_BLUE);
/* 调用相关模块接口完成当前处理步骤。 */
    Indicators_Tick1ms(6100U);assert(leds==0U);
/* 调用相关模块接口完成当前处理步骤。 */
    puts("PASS W1-W5 map to red/blue/green/yellow/white ACK LEDs");
/* 返回当前函数的处理结果。 */
    return 0;
/* 结束当前代码块。 */
}
