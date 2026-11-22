/**
 * @file sound.h
 * @brief 提示音播放接口。
 */
#ifndef LINK_SOUND_H
#define LINK_SOUND_H

#include <stdint.h>

enum
{
    SOUND_BOOT = 1,
    SOUND_PACKET = 2,
    SOUND_FAULT = 3,
    SOUND_PAIR_ENTER = 4,
    SOUND_PAIR_OK = 5,
    SOUND_PAIR_EXIT = 6
};

/**
 * @brief 设置提示音序列并开始播放。
 * @param event 提示音事件类型。
 * @param code 故障码或提示音编号。
 */
void Sound_Play(uint8_t event, uint8_t code);

/**
 * @brief 每毫秒推进提示音状态。
 */
void Sound_Tick1ms(void);

/**
 * @brief 直接控制蜂鸣器 PWM 输出。
 * @param on 置 1 发声，置 0 静音。
 */
void Board_SoundGate(uint8_t on);

#endif
