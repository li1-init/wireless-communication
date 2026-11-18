/* 文件说明：方板灯控模块：启动指示、对码灯超时及收到确认后的短闪。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#include "indicators.h"
#include "board.h"
#include "pairing_tx.h"
static volatile uint8_t timed;
static volatile uint32_t started,duration;
/** @brief 在临界区内设置 LED 及自动熄灭计时。 */
static void TimedLight(uint32_t now,uint32_t ms,uint8_t mask)
{
    uint32_t saved=__get_PRIMASK();
    __disable_irq();
    timed=1U; started=now; duration=ms;
    Board_Leds(mask);
    __set_PRIMASK(saved);
}
/** @brief 取消灯控计时并关闭 LED。 */
void Indicators_Stop(void)
{
    uint32_t saved=__get_PRIMASK();
    __disable_irq();
    timed=0U;
    Board_Leds(0U);
    __set_PRIMASK(saved);
}
/** @brief 依次显示启动灯序列，此阶段使用延时。 */
void Indicators_Startup(void)
{
    uint8_t i;
    const uint8_t lamps[]={LED_MASK_RED,LED_MASK_GREEN,LED_MASK_WHITE};
    Indicators_Stop();
    for(i=0;i<3U;++i) {
        Board_Leds(lamps[i]); Board_DelayMs(200U);
        Board_Leds(0U); Board_DelayMs(200U);
    }
}
/** @brief 显示本地无线初始化完成的绿灯提示。 */
void Indicators_Ready(void)
{
    Board_Leds(LED_MASK_GREEN); Board_DelayMs(500U); Board_Leds(0U);
}
/** @brief 点亮对码灯并设置十秒强制熄灭时间。 */
void Indicators_PairStart(uint32_t now)
{
    TimedLight(now,PAIR_TX_TIMEOUT_MS,LED_MASK_WHITE);
}
/** @brief 点亮短暂绿灯，应用应在匹配接收确认后调用。 */
void Indicators_TxPulse(uint32_t now) { TimedLight(now,120UL,LED_MASK_GREEN); }
/** @brief 根据错误码输出循环红灯闪烁次数。 */
void Indicators_Fault(uint32_t now,uint8_t error)
{
    uint32_t code=error;
    if(!code) { Indicators_Stop(); return; }
    uint32_t phase=now%(code*300UL+1200UL);
    timed=0U;
    Board_Leds((phase<code*300UL && phase%300UL<120UL)?LED_MASK_RED:0U);
}
/** @brief 在 SysTick 中按时熄灯，不依赖主循环是否正在等待无线。 */
void Indicators_Tick1ms(uint32_t now)
{
    if(timed && (uint32_t)(now-started)>=duration) {
        timed=0U;
        Board_Leds(0U);
    }
}
