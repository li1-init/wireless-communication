/* 文件说明：按键模块：消抖、检测按下边沿，并保证每次长按只触发一次。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#include "key_input.h"
static uint8_t previous,stable,pressed,long_fired;
static uint32_t changed[3],down_since[3];
/** @brief 清空按键消抖及长按锁存状态。 */
void Keys_Init(void)
{
    uint8_t i;
    previous=stable=pressed=long_fired=0U;
    for(i=0;i<3U;++i) changed[i]=down_since[i]=0U;
}
/** @brief 更新按键原始状态和稳定状态，生成本轮按下事件。 */
void Keys_Update(uint32_t now,uint8_t raw)
{
    uint8_t i,bit;
    pressed=0U;
    for(i=0;i<3U;++i) {
        bit=(uint8_t)(1U<<i);
        if((raw^previous)&bit) changed[i]=now;
        if(((raw^stable)&bit) && (uint32_t)(now-changed[i])>=25UL) {
            stable^=bit;
            if(stable&bit) {
                pressed|=bit;
                down_since[i]=now;
                long_fired&=(uint8_t)~bit;
            }
        }
    }
    previous=raw;
}
/** @brief 取得本轮消抖后的按下边沿位掩码。 */
uint8_t Keys_Pressed(void) { return pressed; }
/** @brief 检测达到阈值的长按，同一次按住只触发一次。 */
uint8_t Keys_LongPress(uint8_t mask,uint32_t now,uint32_t threshold)
{
    uint8_t i,bit,result=0U;
    for(i=0;i<3U;++i) {
        bit=(uint8_t)(1U<<i);
        if((mask&stable&bit) && !(long_fired&bit) &&
           (uint32_t)(now-down_since[i])>=threshold) {
            long_fired|=bit;
            result|=bit;
        }
    }
    return result;
}
