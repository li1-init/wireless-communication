/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：方形接收板灯控：启动、接收提示、对码和对码成功快闪。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 根据编译条件选择对应实现。 */
#ifndef INDICATORS_H
/* 定义编译期常量或接口宏。 */
#define INDICATORS_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/* 定义编译期常量或接口宏。 */
#define ACK_LED_MS 100UL
/** @brief 依次显示三个启动灯，时序与原方板发送工程一致。 */
/* 执行当前语句。 */
void Indicators_Startup(void);
/** @brief 显示无线初始化完成的绿灯提示。 */
/* 执行当前语句。 */
void Indicators_Ready(void);
/** @brief 关闭全部 LED 并退出对码灯状态。 */
/* 执行当前语句。 */
void Indicators_Stop(void);
/** @brief 进入对码状态，白灯常亮。 */
/* 执行当前语句。 */
void Indicators_PairStart(uint32_t now);
/** @brief 对码成功后白灯快速闪烁两次，然后恢复常亮。 */
/* 执行当前语句。 */
void Indicators_PairSuccess(uint32_t now);
/** @brief 收到有效数据后绿灯亮 1 秒。 */
/* 执行当前语句。 */
void Indicators_RxPulse(uint32_t now);
/** @brief 返回按键对应的彩色反馈灯掩码；普通键回退到原反馈灯组。 */
/* 执行当前语句。 */
uint8_t Indicators_KeyLedMask(uint8_t key_mask);
/** @brief ACK 匹配后点亮对应彩色灯 100 毫秒。 */
/* 执行当前语句。 */
void Indicators_KeyAck(uint32_t now,uint8_t key_mask);
/** @brief 返回接收命令 3～7 对应的彩色反馈灯掩码。 */
/* 执行当前语句。 */
uint8_t Indicators_CommandLedMask(uint8_t command);
/** @brief 接收端处理 W1～W5 命令时点亮对应彩色灯 100 毫秒。 */
/* 执行当前语句。 */
void Indicators_CommandAck(uint32_t now,uint8_t command);
/** @brief 根据错误码输出循环红灯闪烁。 */
/* 执行当前语句。 */
void Indicators_Fault(uint32_t now,uint8_t error);
/** @brief 在 SysTick 中推进灯效计时。 */
/* 执行当前语句。 */
void Indicators_Tick1ms(uint32_t now);
/* 根据编译条件选择对应实现。 */
#endif
