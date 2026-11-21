/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：圆板声音模块：按毫秒推进提示音，避免主循环等待时蜂鸣器持续鸣叫。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 根据编译条件选择对应实现。 */
#ifndef LINK_SOUND_H
/* 定义编译期常量或接口宏。 */
#define LINK_SOUND_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/* 定义本模块使用的数据类型。 */
enum { SOUND_BOOT=1, SOUND_PACKET=2, SOUND_FAULT=3,
/* 更新运行状态或计算结果。 */
       SOUND_PAIR_ENTER=4, SOUND_PAIR_OK=5, SOUND_PAIR_EXIT=6 };
/* Board_Sound wraps Sound_Play in an interrupt-safe critical section.
 * Sound_Tick1ms runs from SysTick so a stalled SPI command cannot sustain a tone. */
/** @brief 设置提示音的发声和静音分段时长，替换当前音序列。 */
/* 执行当前语句。 */
void Sound_Play(uint8_t event,uint8_t code);
/** @brief 每毫秒推进提示音包络，到时关闭 PWM。 */
/* 执行当前语句。 */
void Sound_Tick1ms(void);
/** @brief 直接控制蜂鸣器 PWM 占空比，实现发声或静音。 */
/* 执行当前语句。 */
void Board_SoundGate(uint8_t on);
/* 根据编译条件选择对应实现。 */
#endif
