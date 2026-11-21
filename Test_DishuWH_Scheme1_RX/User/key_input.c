/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：按键模块：消抖、检测按下边沿，并保证每次长按只触发一次。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 引入本模块所需的头文件接口。 */
#include "key_input.h"
/* 执行当前语句。 */
static uint8_t previous,stable,pressed,long_fired;
/* 定义或初始化本模块使用的状态数据。 */
static uint32_t changed[8],down_since[8];
/** @brief 清空按键消抖及长按锁存状态。 */
/* 进入本模块的函数实现。 */
void Keys_Init(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t i;
/* 更新运行状态或计算结果。 */
    previous=stable=pressed=long_fired=0U;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<8U;++i) changed[i]=down_since[i]=0U;
/* 结束当前代码块。 */
}
/** @brief 更新按键原始状态和稳定状态，生成本轮按下事件。 */
/* 进入本模块的函数实现。 */
void Keys_Update(uint32_t now,uint8_t raw)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    uint8_t i,bit;
/* 更新运行状态或计算结果。 */
    pressed=0U;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<8U;++i) {
/* 更新运行状态或计算结果。 */
        bit=(uint8_t)(1U<<i);
/* 根据运行条件选择处理分支。 */
        if((raw^previous)&bit) changed[i]=now;
/* 进入本模块的函数实现。 */
        if(((raw^stable)&bit) && (uint32_t)(now-changed[i])>=25UL) {
/* 执行当前语句。 */
            stable^=bit;
/* 进入本模块的函数实现。 */
            if(stable&bit) {
/* 执行当前语句。 */
                pressed|=bit;
/* 执行当前语句。 */
                down_since[i]=now;
/* 执行当前语句。 */
                long_fired&=(uint8_t)~bit;
/* 结束当前代码块。 */
            }
/* 结束当前代码块。 */
        }
/* 结束当前代码块。 */
    }
/* 更新运行状态或计算结果。 */
    previous=raw;
/* 结束当前代码块。 */
}
/** @brief 取得本轮消抖后的按下边沿位掩码。 */
/* 进入本模块的函数实现。 */
uint8_t Keys_Pressed(void) { return pressed; }
/** @brief 检测达到阈值的长按，同一次按住只触发一次。 */
/* 进入本模块的函数实现。 */
uint8_t Keys_LongPress(uint8_t mask,uint32_t now,uint32_t threshold)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    uint8_t i,bit,result=0U;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<8U;++i) {
/* 更新运行状态或计算结果。 */
        bit=(uint8_t)(1U<<i);
/* 根据运行条件选择处理分支。 */
        if((mask&stable&bit) && !(long_fired&bit) &&
/* 说明本行代码在当前模块中的处理作用。 */
           (uint32_t)(now-down_since[i])>=threshold) {
/* 执行当前语句。 */
            long_fired|=bit;
/* 执行当前语句。 */
            result|=bit;
/* 结束当前代码块。 */
        }
/* 结束当前代码块。 */
    }
/* 返回当前函数的处理结果。 */
    return result;
/* 结束当前代码块。 */
}
