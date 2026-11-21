/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：圆板声音模块：按毫秒推进提示音，避免主循环等待时蜂鸣器持续鸣叫。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 引入本模块所需的头文件接口。 */
#include "sound.h"
/* 定义或初始化本模块使用的状态数据。 */
static volatile uint16_t durations[20];
/* 定义或初始化本模块使用的状态数据。 */
static volatile uint16_t remaining;
/* 执行当前语句。 */
static volatile uint8_t index_now,count;

/** @brief 设置提示音的发声和静音分段时长，替换当前音序列。 */
/* 进入本模块的函数实现。 */
void Sound_Play(uint8_t event,uint8_t code)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t i;
/* 更新运行状态或计算结果。 */
    count=0;remaining=0;index_now=0;
/* 调用相关模块接口完成当前处理步骤。 */
    Board_SoundGate(0);
/* 进入本模块的函数实现。 */
    if(event==SOUND_BOOT) { durations[0]=350;count=1; }
/* 进入本模块的函数实现。 */
    else if(event==SOUND_PACKET) {
/* 执行当前语句。 */
        durations[0]=120;count=1;
/* 说明本行代码在当前模块中的处理作用。 */
    } else if(event==SOUND_PAIR_ENTER) {
/* 执行当前语句。 */
        durations[0]=2000;count=1;
/* 说明本行代码在当前模块中的处理作用。 */
    } else if(event==SOUND_PAIR_OK) {
/* 执行当前语句。 */
        durations[0]=500;count=1;
/* 说明本行代码在当前模块中的处理作用。 */
    } else if(event==SOUND_PAIR_EXIT) {
/* 执行当前语句。 */
        durations[0]=120;durations[1]=100;durations[2]=120;count=3;
/* 说明本行代码在当前模块中的处理作用。 */
    } else if(event==SOUND_FAULT) {
/* 根据运行条件选择处理分支。 */
        if(code<1U||code>9U) code=4U;
/* 执行当前语句。 */
        durations[0]=500;durations[1]=250;
/* 循环处理当前任务，直到满足退出条件。 */
        for(i=0;i<code;++i) {
/* 执行当前语句。 */
            durations[2U+i*2U]=120;
/* 执行当前语句。 */
            durations[3U+i*2U]=180;
/* 结束当前代码块。 */
        }
/* 更新运行状态或计算结果。 */
        count=(uint8_t)(2U*code+1U);
/* 结束当前代码块。 */
    }
/* 进入本模块的函数实现。 */
    if(count) { remaining=durations[0];Board_SoundGate(1); }
/* 结束当前代码块。 */
}
/** @brief 每毫秒推进提示音包络，到时关闭 PWM。 */
/* 进入本模块的函数实现。 */
void Sound_Tick1ms(void)
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(!remaining) return;
/* 根据运行条件选择处理分支。 */
    if(--remaining) return;
/* 更新运行状态或计算结果。 */
    ++index_now;
/* 进入本模块的函数实现。 */
    if(index_now>=count) { Board_SoundGate(0);count=0;return; }
/* 更新运行状态或计算结果。 */
    remaining=durations[index_now];
/* 调用相关模块接口完成当前处理步骤。 */
    Board_SoundGate((index_now&1U)?0U:1U);
/* 结束当前代码块。 */
}
