/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：无线底层接口：声明时基、片选、电源和 SPI 操作函数。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 根据编译条件选择对应实现。 */
#ifndef LINK_RADIO_PORT_H
/* 定义编译期常量或接口宏。 */
#define LINK_RADIO_PORT_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/** @brief 返回毫秒时基；计算时间差使用无符号减法以兼容回绕。 */
/* 执行当前语句。 */
uint32_t Board_Millis(void);
/** @brief 等待指定毫秒数；中断仍可执行，但主循环暂停。 */
/* 执行当前语句。 */
void Board_DelayMs(uint32_t ms);
/** @brief 初始化无线 GPIO、SPI1 以及 SPI DMA 通道。 */
/* 执行当前语句。 */
void Board_RadioInit(void);
/** @brief 配置 SPI1 时钟、极性和数据格式。 */
/* 执行当前语句。 */
void Board_RadioSpeed(uint8_t fast);
/** @brief 控制无线芯片 SPI 片选，保留短暂建立时间。 */
/* 执行当前语句。 */
void Board_Nsel(uint8_t high);
/** @brief 控制无线芯片关断引脚。 */
/* 执行当前语句。 */
void Board_Sdn(uint8_t high);
/** @brief 轮询发送和接收一个 SPI 字节，用于短命令和状态查询。 */
/* 执行当前语句。 */
uint8_t Board_SpiByte(uint8_t tx,uint8_t *rx);
/** @brief 等待 SPI 忙标志清除，确保最后一位真正发出。 */
/* 执行当前语句。 */
uint8_t Board_SpiFinish(void);
/* 根据编译条件选择对应实现。 */
#endif
