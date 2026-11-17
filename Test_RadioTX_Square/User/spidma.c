/* 文件说明：SPI1 DMA 模块：使用普通 DMA 完成一次指定长度的 FIFO 传输；不是硬件循环 DMA。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This module moves complete Si4438 FIFO packets with STM32L1 DMA1. */
#include "spidma.h"
#include "board.h"
volatile SpiDmaStatus g_spi_dma;
static uint8_t dummy_tx=0xFFU,dummy_rx;
/** @brief 关闭 SPI DMA 请求和两个 DMA 通道，清除完成及错误标志。 */
static void Stop(void)
{
    SPI1->CR2&=(uint16_t)~(SPI_CR2_TXDMAEN|SPI_CR2_RXDMAEN);
    DMA1_Channel3->CCR&=(uint16_t)~DMA_CCR3_EN;
    DMA1_Channel2->CCR&=(uint16_t)~DMA_CCR2_EN;
    DMA1->IFCR=DMA_IFCR_CGIF2|DMA_IFCR_CGIF3;
}
/** @brief 开启 DMA1 时钟，把 SPI1 收发 DMA 通道置于停止状态。 */
void SpiDma_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
    Stop();
}
/** @brief 双通道 DMA 搬运指定字节数，等待完成；错误或超时清理后返回 0。 */
uint8_t SpiDma_Transfer(const uint8_t *tx,uint8_t *rx,uint16_t size)
{
    uint32_t start,flags;
    volatile uint32_t discard;
    if(!size || (!tx && !rx)) return 0U;
    Stop();
    if(SPI1->SR&(SPI_SR_RXNE|SPI_SR_OVR)) {
        discard=SPI1->DR; discard=SPI1->SR; (void)discard;
    }
    /* STM32L151: SPI1_RX = DMA1 channel 2, SPI1_TX = DMA1 channel 3.
     * Both channels run even for one-way FIFO data, preventing RX overrun. */
    DMA1_Channel2->CPAR=(uint32_t)&SPI1->DR;
    DMA1_Channel2->CMAR=(uint32_t)(rx ? rx : &dummy_rx);
    DMA1_Channel2->CNDTR=size;
    DMA1_Channel2->CCR=DMA_CCR2_PL_1|(rx ? DMA_CCR2_MINC : 0U);
    DMA1_Channel3->CPAR=(uint32_t)&SPI1->DR;
    DMA1_Channel3->CMAR=(uint32_t)(tx ? tx : &dummy_tx);
    DMA1_Channel3->CNDTR=size;
    DMA1_Channel3->CCR=DMA_CCR3_PL_1|DMA_CCR3_DIR|(tx ? DMA_CCR3_MINC : 0U);
    g_spi_dma.last_tx_address=DMA1_Channel3->CMAR;
    g_spi_dma.last_rx_address=DMA1_Channel2->CMAR;
    g_spi_dma.last_size=size;
    __DMB();
    start=Board_Millis();
    DMA1_Channel2->CCR|=DMA_CCR2_EN;
    DMA1_Channel3->CCR|=DMA_CCR3_EN;
    SPI1->CR2|=SPI_CR2_RXDMAEN|SPI_CR2_TXDMAEN;
    for(;;) {
        flags=DMA1->ISR;
        g_spi_dma.last_flags=flags;
        if(flags&(DMA_ISR_TEIF2|DMA_ISR_TEIF3)) {
            ++g_spi_dma.errors; Stop(); return 0U;
        }
        if((flags&(DMA_ISR_TCIF2|DMA_ISR_TCIF3))==
           (DMA_ISR_TCIF2|DMA_ISR_TCIF3)) break;
        if((uint32_t)(Board_Millis()-start)>=10UL) {
            ++g_spi_dma.timeouts; Stop(); return 0U;
        }
    }
    Stop();
    __DMB();
    if(!Board_SpiFinish()) { ++g_spi_dma.timeouts; return 0U; }
    ++g_spi_dma.completed;
    if(tx) ++g_spi_dma.tx_packets;
    if(rx) ++g_spi_dma.rx_packets;
    return 1U;
}
