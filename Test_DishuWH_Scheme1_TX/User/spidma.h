/* 文件说明：SPI1 DMA 模块：使用普通 DMA 完成一次指定长度的 FIFO 传输；不是硬件循环 DMA。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This file defines the SPI1 DMA transfer status and packet transfer API. */
#ifndef SPIDMA_H
#define SPIDMA_H
#include <stdint.h>
typedef struct {
    uint32_t completed,tx_packets,rx_packets,timeouts,errors;
    uint32_t last_tx_address,last_rx_address,last_size,last_flags;
} SpiDmaStatus;
extern volatile SpiDmaStatus g_spi_dma;
/** @brief 开启 DMA1 时钟，把 SPI1 收发 DMA 通道置于停止状态。 */
void SpiDma_Init(void);
/** @brief 双通道 DMA 搬运指定字节数，等待完成；错误或超时清理后返回 0。 */
uint8_t SpiDma_Transfer(const uint8_t *tx,uint8_t *rx,uint16_t size);
#endif
