/**
 * @file spidma.c
 * @brief SPI1 DMA 传输实现。
 */
#include "spidma.h"
#include "board.h"

volatile SpiDmaStatus g_spi_dma;

/**
 * @brief SPI DMA 的空字节缓冲区。
 */
typedef struct
{
    uint8_t tx;
    uint8_t rx;
} SpiDmaDummyBuffer;

static SpiDmaDummyBuffer dma_dummy =
{
    0xFFU,
    0U
};

/**
 * @brief 关闭 SPI DMA 请求和两个 DMA 通道。
 */
static void Stop(void)
{
    SPI1->CR2 &= (uint16_t)~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
    DMA1_Channel3->CCR &= (uint16_t)~DMA_CCR3_EN;
    DMA1_Channel2->CCR &= (uint16_t)~DMA_CCR2_EN;
    DMA1->IFCR = DMA_IFCR_CGIF2 | DMA_IFCR_CGIF3;
}

/**
 * @brief 开启 DMA1 时钟并停止 SPI1 DMA 通道。
 */
void SpiDma_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    Stop();
}

/**
 * @brief 使用双通道 DMA 完成一次 SPI 收发。
 * @param tx 待发送缓冲区，可为空。
 * @param rx 接收缓冲区，可为空。
 * @param size 传输字节数。
 * @return 传输成功时返回 1，失败或超时时返回 0。
 */
uint8_t SpiDma_Transfer(const uint8_t *tx,
                        uint8_t *rx,
                        uint16_t size)
{
    uint32_t start;
    uint32_t flags;
    volatile uint32_t discard;

    if (!size || (!tx && !rx))
    {
        return 0U;
    }

    Stop();

    if (SPI1->SR & (SPI_SR_RXNE | SPI_SR_OVR))
    {
        discard = SPI1->DR;
        discard = SPI1->SR;
        (void)discard;
    }

    DMA1_Channel2->CPAR = (uint32_t)&SPI1->DR;
    DMA1_Channel2->CMAR =
        (uint32_t)(rx ? rx : &dma_dummy.rx);
    DMA1_Channel2->CNDTR = size;
    DMA1_Channel2->CCR =
        DMA_CCR2_PL_1 | (rx ? DMA_CCR2_MINC : 0U);

    DMA1_Channel3->CPAR = (uint32_t)&SPI1->DR;
    DMA1_Channel3->CMAR =
        (uint32_t)(tx ? tx : &dma_dummy.tx);
    DMA1_Channel3->CNDTR = size;
    DMA1_Channel3->CCR =
        DMA_CCR3_PL_1 |
        DMA_CCR3_DIR |
        (tx ? DMA_CCR3_MINC : 0U);

    g_spi_dma.last_tx_address = DMA1_Channel3->CMAR;
    g_spi_dma.last_rx_address = DMA1_Channel2->CMAR;
    g_spi_dma.last_size = size;
    __DMB();
    start = Board_Millis();

    DMA1_Channel2->CCR |= DMA_CCR2_EN;
    DMA1_Channel3->CCR |= DMA_CCR3_EN;
    SPI1->CR2 |= SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN;

    for (;;)
    {
        flags = DMA1->ISR;
        g_spi_dma.last_flags = flags;

        if (flags & (DMA_ISR_TEIF2 | DMA_ISR_TEIF3))
        {
            ++g_spi_dma.errors;
            Stop();
            return 0U;
        }

        if ((flags & (DMA_ISR_TCIF2 | DMA_ISR_TCIF3)) ==
            (DMA_ISR_TCIF2 | DMA_ISR_TCIF3))
        {
            break;
        }

        if ((uint32_t)(Board_Millis() - start) >= 10UL)
        {
            ++g_spi_dma.timeouts;
            Stop();
            return 0U;
        }
    }

    Stop();
    __DMB();

    if (!Board_SpiFinish())
    {
        ++g_spi_dma.timeouts;
        return 0U;
    }

    ++g_spi_dma.completed;

    if (tx)
    {
        ++g_spi_dma.tx_packets;
    }

    if (rx)
    {
        ++g_spi_dma.rx_packets;
    }

    return 1U;
}
