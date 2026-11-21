/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：SPI1 DMA 模块：使用普通 DMA 完成一次指定长度的 FIFO 传输；不是硬件循环 DMA。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This module moves complete Si4438 FIFO packets with STM32L1 DMA1. */
/* 引入本模块所需的头文件接口。 */
#include "spidma.h"
/* 引入本模块所需的头文件接口。 */
#include "board.h"
/* 定义或初始化本模块使用的状态数据。 */
volatile SpiDmaStatus g_spi_dma;
/* 定义或初始化本模块使用的状态数据。 */
static uint8_t dummy_tx=0xFFU,dummy_rx;
/** @brief 关闭 SPI DMA 请求和两个 DMA 通道，清除完成及错误标志。 */
/* 进入本模块的函数实现。 */
static void Stop(void)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    SPI1->CR2&=(uint16_t)~(SPI_CR2_TXDMAEN|SPI_CR2_RXDMAEN);
/* 执行当前语句。 */
    DMA1_Channel3->CCR&=(uint16_t)~DMA_CCR3_EN;
/* 执行当前语句。 */
    DMA1_Channel2->CCR&=(uint16_t)~DMA_CCR2_EN;
/* 更新运行状态或计算结果。 */
    DMA1->IFCR=DMA_IFCR_CGIF2|DMA_IFCR_CGIF3;
/* 结束当前代码块。 */
}
/** @brief 开启 DMA1 时钟，把 SPI1 收发 DMA 通道置于停止状态。 */
/* 进入本模块的函数实现。 */
void SpiDma_Init(void)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
/* 调用相关模块接口完成当前处理步骤。 */
    Stop();
/* 结束当前代码块。 */
}
/** @brief 双通道 DMA 搬运指定字节数，等待完成；错误或超时清理后返回 0。 */
/* 进入本模块的函数实现。 */
uint8_t SpiDma_Transfer(const uint8_t *tx,uint8_t *rx,uint16_t size)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    uint32_t start,flags;
/* 定义或初始化本模块使用的状态数据。 */
    volatile uint32_t discard;
/* 根据运行条件选择处理分支。 */
    if(!size || (!tx && !rx)) return 0U;
/* 调用相关模块接口完成当前处理步骤。 */
    Stop();
/* 进入本模块的函数实现。 */
    if(SPI1->SR&(SPI_SR_RXNE|SPI_SR_OVR)) {
/* 更新运行状态或计算结果。 */
        discard=SPI1->DR; discard=SPI1->SR; (void)discard;
/* 结束当前代码块。 */
    }
    /* STM32L151: SPI1_RX = DMA1 channel 2, SPI1_TX = DMA1 channel 3.
     * Both channels run even for one-way FIFO data, preventing RX overrun. */
/* 更新运行状态或计算结果。 */
    DMA1_Channel2->CPAR=(uint32_t)&SPI1->DR;
/* 更新运行状态或计算结果。 */
    DMA1_Channel2->CMAR=(uint32_t)(rx ? rx : &dummy_rx);
/* 更新运行状态或计算结果。 */
    DMA1_Channel2->CNDTR=size;
/* 更新运行状态或计算结果。 */
    DMA1_Channel2->CCR=DMA_CCR2_PL_1|(rx ? DMA_CCR2_MINC : 0U);
/* 更新运行状态或计算结果。 */
    DMA1_Channel3->CPAR=(uint32_t)&SPI1->DR;
/* 更新运行状态或计算结果。 */
    DMA1_Channel3->CMAR=(uint32_t)(tx ? tx : &dummy_tx);
/* 更新运行状态或计算结果。 */
    DMA1_Channel3->CNDTR=size;
/* 更新运行状态或计算结果。 */
    DMA1_Channel3->CCR=DMA_CCR3_PL_1|DMA_CCR3_DIR|(tx ? DMA_CCR3_MINC : 0U);
/* 执行当前语句。 */
    g_spi_dma.last_tx_address=DMA1_Channel3->CMAR;
/* 执行当前语句。 */
    g_spi_dma.last_rx_address=DMA1_Channel2->CMAR;
/* 执行当前语句。 */
    g_spi_dma.last_size=size;
/* 调用相关模块接口完成当前处理步骤。 */
    __DMB();
/* 更新运行状态或计算结果。 */
    start=Board_Millis();
/* 执行当前语句。 */
    DMA1_Channel2->CCR|=DMA_CCR2_EN;
/* 执行当前语句。 */
    DMA1_Channel3->CCR|=DMA_CCR3_EN;
/* 执行当前语句。 */
    SPI1->CR2|=SPI_CR2_RXDMAEN|SPI_CR2_TXDMAEN;
/* 循环处理当前任务，直到满足退出条件。 */
    for(;;) {
/* 更新运行状态或计算结果。 */
        flags=DMA1->ISR;
/* 执行当前语句。 */
        g_spi_dma.last_flags=flags;
/* 进入本模块的函数实现。 */
        if(flags&(DMA_ISR_TEIF2|DMA_ISR_TEIF3)) {
/* 更新运行状态或计算结果。 */
            ++g_spi_dma.errors; Stop(); return 0U;
/* 结束当前代码块。 */
        }
/* 根据运行条件选择处理分支。 */
        if((flags&(DMA_ISR_TCIF2|DMA_ISR_TCIF3))==
/* 执行当前语句。 */
           (DMA_ISR_TCIF2|DMA_ISR_TCIF3)) break;
/* 进入本模块的函数实现。 */
        if((uint32_t)(Board_Millis()-start)>=10UL) {
/* 更新运行状态或计算结果。 */
            ++g_spi_dma.timeouts; Stop(); return 0U;
/* 结束当前代码块。 */
        }
/* 结束当前代码块。 */
    }
/* 调用相关模块接口完成当前处理步骤。 */
    Stop();
/* 调用相关模块接口完成当前处理步骤。 */
    __DMB();
/* 进入本模块的函数实现。 */
    if(!Board_SpiFinish()) { ++g_spi_dma.timeouts; return 0U; }
/* 更新运行状态或计算结果。 */
    ++g_spi_dma.completed;
/* 根据运行条件选择处理分支。 */
    if(tx) ++g_spi_dma.tx_packets;
/* 根据运行条件选择处理分支。 */
    if(rx) ++g_spi_dma.rx_packets;
/* 返回当前函数的处理结果。 */
    return 1U;
/* 结束当前代码块。 */
}
