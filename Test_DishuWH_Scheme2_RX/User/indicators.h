/**
 * @file indicators.h
 * @brief LED 指示灯控制接口。
 */
#ifndef INDICATORS_H
#define INDICATORS_H

#include <stdint.h>

#define ACK_LED_MS 100UL

/**
 * @brief 依次显示启动灯。
 */
void Indicators_Startup(void);

/**
 * @brief 显示无线初始化完成提示。
 */
void Indicators_Ready(void);

/**
 * @brief 关闭全部 LED 并退出配对灯效。
 */
void Indicators_Stop(void);

/**
 * @brief 进入配对状态并点亮配对指示灯。
 * @param now 当前系统毫秒时间戳。
 */
void Indicators_PairStart(uint32_t now);

/**
 * @brief 显示配对成功灯效。
 * @param now 当前系统毫秒时间戳。
 */
void Indicators_PairSuccess(uint32_t now);

/**
 * @brief 显示接收到有效数据的灯效。
 * @param now 当前系统毫秒时间戳。
 */
void Indicators_RxPulse(uint32_t now);

/**
 * @brief 将按键位映射为反馈灯掩码。
 * @param key_mask 按键位掩码。
 * @return 对应的灯掩码。
 */
uint8_t Indicators_KeyLedMask(uint8_t key_mask);

/**
 * @brief ACK 匹配后点亮对应反馈灯。
 * @param now 当前系统毫秒时间戳。
 * @param key_mask 按键位掩码。
 */
void Indicators_KeyAck(uint32_t now, uint8_t key_mask);

/**
 * @brief 将接收命令映射为反馈灯掩码。
 * @param command 协议命令编号。
 * @return 对应的灯掩码。
 */
uint8_t Indicators_CommandLedMask(uint8_t command);

/**
 * @brief 接收端处理命令时点亮对应反馈灯。
 * @param now 当前系统毫秒时间戳。
 * @param command 协议命令编号。
 */
void Indicators_CommandAck(uint32_t now, uint8_t command);

/**
 * @brief 根据错误码输出故障灯效。
 * @param now 当前系统毫秒时间戳。
 * @param error 无线驱动错误码。
 */
void Indicators_Fault(uint32_t now, uint8_t error);

/**
 * @brief 在 SysTick 中推进灯效计时。
 * @param now 当前系统毫秒时间戳。
 */
void Indicators_Tick1ms(uint32_t now);

#endif
