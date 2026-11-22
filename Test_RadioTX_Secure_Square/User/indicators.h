/* 文件说明：方板灯控模块：启动指示、对码灯超时及收到确认后的短闪。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef INDICATORS_H
#define INDICATORS_H
#include <stdint.h>
/** @brief 依次显示启动灯序列，此阶段使用延时。 */
void Indicators_Startup(void);
/** @brief 显示本地无线初始化完成的绿灯提示。 */
void Indicators_Ready(void);
/** @brief 点亮对码灯并设置配对窗口强制熄灭时间。 */
void Indicators_PairStart(uint32_t now);
/** @brief 取消灯控计时并关闭 LED。 */
void Indicators_Stop(void);
/** @brief 收到匹配的数据确认后点亮绿灯 1 秒。 */
void Indicators_TxPulse(uint32_t now);
/** @brief 根据错误码输出循环红灯闪烁次数。 */
void Indicators_Fault(uint32_t now,uint8_t error);
/** @brief 在 SysTick 中按时熄灯，不依赖主循环是否正在等待无线。 */
void Indicators_Tick1ms(uint32_t now);
#endif
