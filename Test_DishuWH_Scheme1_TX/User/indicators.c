/* 文件说明：方形接收板灯控模块，用非阻塞计时实现收包和对码灯效。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#include "indicators.h"
#include "board.h"
#include "key_input.h"
static volatile uint8_t timed,pair_active,pair_flash,flash_steps;
static volatile uint32_t started,duration,pair_started;
/** @brief 在临界区内设置 LED 及自动熄灭计时。 */
static void TimedLight(uint32_t now,uint32_t ms,uint8_t mask)
{
    uint32_t saved=__get_PRIMASK();
    __disable_irq();
    timed=1U; started=now; duration=ms;
    pair_active=0U; pair_flash=0U;
    Board_Leds(mask);
    __set_PRIMASK(saved);
}
/** @brief 取消全部灯控状态并关闭 LED。 */
void Indicators_Stop(void)
{
    uint32_t saved=__get_PRIMASK();
    __disable_irq();
    timed=0U;
    pair_active=0U;
    pair_flash=0U;
    Board_Leds(0U);
    __set_PRIMASK(saved);
}
/** @brief 依次显示启动灯，时序与原方板发送工程一致。 */
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
    Indicators_Stop();
    Board_Leds(LED_MASK_GREEN); Board_DelayMs(500U); Board_Leds(0U);
}
/** @brief 进入对码状态，白灯常亮。 */
void Indicators_PairStart(uint32_t now)
{
    uint32_t saved=__get_PRIMASK();
    __disable_irq();
    timed=0U;
    pair_active=1U;
    pair_flash=0U;
    pair_started=now;
    Board_Leds(LED_MASK_WHITE);
    __set_PRIMASK(saved);
}
/** @brief 对码成功后白灯快速闪烁两次，然后恢复常亮。 */
void Indicators_PairSuccess(uint32_t now)
{
    uint32_t saved=__get_PRIMASK();
    __disable_irq();
    timed=0U;
    pair_active=1U;
    pair_flash=1U;
    flash_steps=0U;
    pair_started=now;
    Board_Leds(0U);
    __set_PRIMASK(saved);
}
/** @brief 收到有效数据后绿灯亮 1 秒。 */
void Indicators_RxPulse(uint32_t now)
{
    TimedLight(now,TX_SUCCESS_LED_MS,LED_MASK_RED|LED_MASK_GREEN|LED_MASK_WHITE);
}
/** @brief 将 W1-W5 一一映射到红、蓝、绿、黄、白反馈灯。 */
uint8_t Indicators_KeyLedMask(uint8_t key_mask)
{
    if(key_mask&KEY_W1_MASK)return LED_MASK_RED;
    if(key_mask&KEY_W2_MASK)return LED_MASK_BLUE;
    if(key_mask&KEY_W3_MASK)return LED_MASK_GREEN;
    if(key_mask&KEY_W4_MASK)return LED_MASK_YELLOW;
    if(key_mask&KEY_W5_MASK)return LED_MASK_WHITE;
    return LED_MASK_RED|LED_MASK_GREEN|LED_MASK_WHITE;
}
/** @brief ACK 匹配后仅点亮本次按键对应的灯 100 毫秒。 */
void Indicators_KeyAck(uint32_t now,uint8_t key_mask)
{
    TimedLight(now,ACK_LED_MS,Indicators_KeyLedMask(key_mask));
}
/** @brief 根据错误码输出循环红灯闪烁。 */
void Indicators_Fault(uint32_t now,uint8_t error)
{
    uint32_t code=error;
    if(!code) { Indicators_Stop(); return; }
    uint32_t phase=now%(code*300UL+1200UL);
    timed=0U;
    pair_active=0U;
    pair_flash=0U;
    Board_Leds((phase<code*300UL && phase%300UL<120UL)?LED_MASK_RED:0U);
}
/** @brief 在 SysTick 中按时熄灯，并维持对码常亮或成功快闪。 */
void Indicators_Tick1ms(uint32_t now)
{
    if(timed && (uint32_t)(now-started)>=duration) {
        timed=0U;
        Board_Leds(0U);
    }
    if(pair_active) {
        if(!pair_flash) Board_Leds(LED_MASK_WHITE);
        else if((uint32_t)(now-pair_started)>=100UL) {
            pair_started+=100UL;
            ++flash_steps;
            if(flash_steps>=5U) {
                pair_flash=0U;
                Board_Leds(LED_MASK_WHITE);
            } else Board_Leds((flash_steps==1U || flash_steps==3U)?
                              LED_MASK_WHITE:0U);
        }
    }
}
