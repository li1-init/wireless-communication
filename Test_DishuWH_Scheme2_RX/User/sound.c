/**
 * @file sound.c
 * @brief 提示音播放实现。
 */
#include "sound.h"

/**
 * @brief 提示音播放状态。
 */
typedef struct
{
    volatile uint16_t durations[20];
    volatile uint16_t remaining;
    volatile uint8_t index_now;
    volatile uint8_t count;
} SoundState;

static SoundState sound_state;

/**
 * @brief 设置提示音序列并开始播放。
 * @param event 提示音事件类型。
 * @param code 故障码或提示音编号。
 */
void Sound_Play(uint8_t event, uint8_t code)
{
    uint8_t index;

    sound_state.count = 0U;
    sound_state.remaining = 0U;
    sound_state.index_now = 0U;
    Board_SoundGate(0U);

    if (event == SOUND_BOOT)
    {
        sound_state.durations[0] = 350U;
        sound_state.count = 1U;
    }
    else if (event == SOUND_PACKET)
    {
        sound_state.durations[0] = 120U;
        sound_state.count = 1U;
    }
    else if (event == SOUND_PAIR_ENTER)
    {
        sound_state.durations[0] = 2000U;
        sound_state.count = 1U;
    }
    else if (event == SOUND_PAIR_OK)
    {
        sound_state.durations[0] = 500U;
        sound_state.count = 1U;
    }
    else if (event == SOUND_PAIR_EXIT)
    {
        sound_state.durations[0] = 120U;
        sound_state.durations[1] = 100U;
        sound_state.durations[2] = 120U;
        sound_state.count = 3U;
    }
    else if (event == SOUND_FAULT)
    {
        if (code < 1U || code > 9U)
        {
            code = 4U;
        }

        sound_state.durations[0] = 500U;
        sound_state.durations[1] = 250U;

        for (index = 0U; index < code; ++index)
        {
            sound_state.durations[2U + index * 2U] = 120U;
            sound_state.durations[3U + index * 2U] = 180U;
        }

        sound_state.count = (uint8_t)(2U * code + 1U);
    }

    if (sound_state.count)
    {
        sound_state.remaining = sound_state.durations[0];
        Board_SoundGate(1U);
    }
}

/**
 * @brief 每毫秒推进提示音状态。
 */
void Sound_Tick1ms(void)
{
    if (!sound_state.remaining)
    {
        return;
    }

    if (--sound_state.remaining)
    {
        return;
    }

    ++sound_state.index_now;

    if (sound_state.index_now >= sound_state.count)
    {
        Board_SoundGate(0U);
        sound_state.count = 0U;
        return;
    }

    sound_state.remaining =
        sound_state.durations[sound_state.index_now];
    Board_SoundGate((sound_state.index_now & 1U) ? 0U : 1U);
}
