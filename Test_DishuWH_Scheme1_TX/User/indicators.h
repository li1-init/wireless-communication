/* 文件说明：方形接收板灯控：启动、接收提示、对码和对码成功快闪。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef INDICATORS_H
#define INDICATORS_H
#include <stdint.h>
#define TX_SUCCESS_LED_MS 100UL
/** @brief 依次显示三个启动灯，时序与原方板发送工程一致。 */
void Indicators_Startup(void);
/** @brief 显示无线初始化完成的绿灯提示。 */
void Indicators_Ready(void);
/** @brief 关闭全部 LED 并退出对码灯状态。 */
void Indicators_Stop(void);
/** @brief 进入对码状态，白灯常亮。 */
void Indicators_PairStart(uint32_t now);
/** @brief 对码成功后白灯快速闪烁两次，然后恢复常亮。 */
void Indicators_PairSuccess(uint32_t now);
/** @brief 收到有效数据后绿灯亮 1 秒。 */
void Indicators_RxPulse(uint32_t now);
/** @brief 根据错误码输出循环红灯闪烁。 */
void Indicators_Fault(uint32_t now,uint8_t error);
/** @brief 在 SysTick 中推进灯效计时。 */
void Indicators_Tick1ms(uint32_t now);
#endif
