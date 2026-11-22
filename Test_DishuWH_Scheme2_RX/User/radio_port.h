/**
 * @file radio_port.h
 * @brief 无线驱动使用的板级端口接口。
 */
#ifndef LINK_RADIO_PORT_H
#define LINK_RADIO_PORT_H

#include <stdint.h>

/**
 * @brief 返回系统毫秒时基。
 * @return 当前毫秒计数。
 */
uint32_t Board_Millis(void);

/**
 * @brief 延时指定毫秒数。
 * @param ms 延时时间，单位为毫秒。
 */
void Board_DelayMs(uint32_t ms);

/**
 * @brief 初始化无线 GPIO、SPI1 和 SPI DMA。
 */
void Board_RadioInit(void);

/**
 * @brief 配置 SPI1 速度和通信格式。
 * @param fast 是否使用高速配置。
 */
void Board_RadioSpeed(uint8_t fast);

/**
 * @brief 控制无线芯片 SPI 片选。
 * @param high 置 1 释放片选，置 0 选中芯片。
 */
void Board_Nsel(uint8_t high);

/**
 * @brief 控制无线芯片关断引脚。
 * @param high 置 1 进入关断，置 0 正常工作。
 */
void Board_Sdn(uint8_t high);

/**
 * @brief 轮询发送和接收一个 SPI 字节。
 * @param tx 待发送字节。
 * @param rx 接收字节输出地址。
 * @return 传输成功时返回 1，否则返回 0。
 */
uint8_t Board_SpiByte(uint8_t tx, uint8_t *rx);

/**
 * @brief 等待 SPI 忙标志清除。
 * @return SPI 已完成时返回 1，否则返回 0。
 */
uint8_t Board_SpiFinish(void);

#endif
