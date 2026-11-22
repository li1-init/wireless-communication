/**
 * @file spidma.h
 * @brief SPI1 DMA 传输接口。
 */
#ifndef SPIDMA_H
#define SPIDMA_H

#include <stdint.h>

/**
 * @brief SPI DMA 运行统计信息。
 */
typedef struct
{
    uint32_t completed;
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t timeouts;
    uint32_t errors;
    uint32_t last_tx_address;
    uint32_t last_rx_address;
    uint32_t last_size;
    uint32_t last_flags;
} SpiDmaStatus;

extern volatile SpiDmaStatus g_spi_dma;

/**
 * @brief 开启 DMA1 时钟并停止 SPI1 DMA 通道。
 */
void SpiDma_Init(void);

/**
 * @brief 使用双通道 DMA 完成一次 SPI 收发。
 * @param tx 待发送缓冲区，可为空。
 * @param rx 接收缓冲区，可为空。
 * @param size 传输字节数。
 * @return 传输成功时返回 1，失败或超时时返回 0。
 */
uint8_t SpiDma_Transfer(const uint8_t *tx,
                        uint8_t *rx,
                        uint16_t size);

#endif
