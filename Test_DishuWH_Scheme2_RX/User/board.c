/**
 * @file board.c
 * @brief 板级时钟、GPIO、按键、LED 和 SPI 驱动实现。
 */
#include "board.h"
#include "indicators.h"
#include "spidma.h"

uint32_t SystemCoreClock = 2097152UL;

static volatile uint32_t ticks;

/**
 * @brief 设置复位早期的中断向量表地址。
 */
void SystemInit(void)
{
    SCB->VTOR = FLASH_BASE;
}

/**
 * @brief 根据 RCC 当前配置更新系统时钟频率变量。
 */
void SystemCoreClockUpdate(void)
{
    RCC_ClocksTypeDef clocks;

    RCC_GetClocksFreq(&clocks);
    SystemCoreClock = clocks.HCLK_Frequency;
}

/**
 * @brief 每毫秒递增时基并推进灯效或提示音计时。
 */
void SysTick_Handler(void)
{
    ++ticks;
    Indicators_Tick1ms(ticks);

#if BOARD_HAS_BUZZER
    Sound_Tick1ms();
#endif
}

/**
 * @brief 返回系统毫秒时基。
 * @return 当前毫秒计数。
 */
uint32_t Board_Millis(void)
{
    return ticks;
}

/**
 * @brief 延时指定毫秒数。
 * @param ms 延时时间，单位为毫秒。
 */
void Board_DelayMs(uint32_t ms)
{
    uint32_t start;

    start = ticks;

    while ((uint32_t)(ticks - start) < ms)
    {
    }
}

/**
 * @brief 配置一个 GPIO 端口上的指定引脚。
 * @param port GPIO 端口。
 * @param pins 要配置的引脚。
 * @param mode GPIO 工作模式。
 * @param pull GPIO 上拉下拉配置。
 */
static void PinInit(GPIO_TypeDef *port,
                    uint16_t pins,
                    GPIOMode_TypeDef mode,
                    GPIOPuPd_TypeDef pull)
{
    GPIO_InitTypeDef gpio;

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = pins;
    gpio.GPIO_Mode = mode;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_PuPd = pull;
    GPIO_Init(port, &gpio);
}

/**
 * @brief 配置内部 HSI 和总线时钟。
 * @return 时钟配置成功时返回 1，否则返回 0。
 */
static uint8_t ClockInit(void)
{
    uint32_t guard;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_VoltageScalingConfig(PWR_VoltageScaling_Range1);

    for (guard = 0U; guard < 200000UL; ++guard)
    {
        if (PWR_GetFlagStatus(PWR_FLAG_VOS) == RESET)
        {
            break;
        }
    }

    if (guard == 200000UL)
    {
        return 0U;
    }

    FLASH_SetLatency(FLASH_Latency_1);
    RCC_HSICmd(ENABLE);

    for (guard = 0U; guard < 200000UL; ++guard)
    {
        if (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == SET)
        {
            break;
        }
    }

    if (guard == 200000UL)
    {
        return 0U;
    }

    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

    for (guard = 0U; guard < 200000UL; ++guard)
    {
        if (RCC_GetSYSCLKSource() == 0x04U)
        {
            break;
        }
    }

    if (guard == 200000UL)
    {
        return 0U;
    }

    SystemCoreClockUpdate();
    return SystemCoreClock == 16000000UL ? 1U : 0U;
}

#if BOARD_HAS_BUZZER
/**
 * @brief 初始化 TIM4_CH3 的蜂鸣器 PWM。
 */
static void BuzzerInit(void)
{
    TIM_TimeBaseInitTypeDef timer;
    TIM_OCInitTypeDef channel;
    RCC_ClocksTypeDef clocks;
    uint32_t timer_hz;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
    PinInit(BUZZER_PORT,
            BUZZER_PIN,
            GPIO_Mode_OUT,
            GPIO_PuPd_NOPULL);
    RCC_GetClocksFreq(&clocks);
    timer_hz = clocks.PCLK1_Frequency;

    if (clocks.PCLK1_Frequency != clocks.HCLK_Frequency)
    {
        timer_hz *= 2U;
    }

    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler = (uint16_t)(timer_hz / 1000000UL - 1UL);
    timer.TIM_Period = 343U;
    TIM_TimeBaseInit(TIM4, &timer);

    TIM_OCStructInit(&channel);
    channel.TIM_OCMode = TIM_OCMode_PWM1;
    channel.TIM_OutputState = TIM_OutputState_Enable;
    channel.TIM_OCPolarity = TIM_OCPolarity_High;
    channel.TIM_Pulse = 0U;
    TIM_OC3Init(TIM4, &channel);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_GenerateEvent(TIM4, TIM_EventSource_Update);
    GPIO_PinAFConfig(BUZZER_PORT,
                     BUZZER_PIN_SOURCE,
                     GPIO_AF_TIM4);
    PinInit(BUZZER_PORT,
            BUZZER_PIN,
            GPIO_Mode_AF,
            GPIO_PuPd_NOPULL);
    TIM_Cmd(TIM4, ENABLE);
}
#endif

/**
 * @brief 直接控制蜂鸣器 PWM 输出。
 * @param on 置 1 发声，置 0 静音。
 */
void Board_SoundGate(uint8_t on)
{
#if BOARD_HAS_BUZZER
    TIM4->CCR3 = on ? 172U : 0U;
#else
    (void)on;
#endif
}

/**
 * @brief 在临界区内启动提示音。
 * @param event 提示音事件类型。
 * @param code 故障码或提示音编号。
 */
void Board_Sound(uint8_t event, uint8_t code)
{
#if BOARD_HAS_BUZZER
    uint32_t saved_primask;

    saved_primask = __get_PRIMASK();
    __disable_irq();
    Sound_Play(event, code);
    __set_PRIMASK(saved_primask);
#else
    (void)event;
    (void)code;
#endif
}

/**
 * @brief 初始化当前板型的时钟、按键、提示器件和毫秒时基。
 * @return 初始化成功时返回 1，否则返回 0。
 */
uint8_t Board_Init(void)
{
    uint8_t clock_ok;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA |
                          RCC_AHBPeriph_GPIOB,
                          ENABLE);

#if BOARD_HAS_LEDS
    Board_Leds(0U);
    PinInit(LED_RED_PORT,
            LED_RED_PIN,
            GPIO_Mode_OUT,
            GPIO_PuPd_NOPULL);
    PinInit(LED_GREEN_PORT,
            LED_GREEN_PIN,
            GPIO_Mode_OUT,
            GPIO_PuPd_NOPULL);
#if BOARD_HAS_WHITE
    PinInit(LED_WHITE_PORT,
            LED_WHITE_PIN,
            GPIO_Mode_OUT,
            GPIO_PuPd_NOPULL);
#endif
    PinInit(LED_BLUE_PORT,
            LED_BLUE_PIN,
            GPIO_Mode_OUT,
            GPIO_PuPd_NOPULL);
    PinInit(LED_YELLOW_PORT,
            LED_YELLOW_PIN,
            GPIO_Mode_OUT,
            GPIO_PuPd_NOPULL);
#endif

    PinInit(KEY_SWITCH_PORT,
            KEY_SWITCH_PIN,
            GPIO_Mode_IN,
            GPIO_PuPd_NOPULL);

#if BOARD_HAS_USER
    PinInit(KEY_USER_PORT,
            KEY_USER_PIN,
            GPIO_Mode_IN,
            GPIO_PuPd_NOPULL);
#endif

#if BOARD_HAS_CODE
    PinInit(KEY_CODE_PORT,
            KEY_CODE_PIN,
            GPIO_Mode_IN,
            GPIO_PuPd_NOPULL);
#endif

    PinInit(KEY_W1_PORT,
            KEY_W1_PIN,
            GPIO_Mode_IN,
            GPIO_PuPd_DOWN);
    PinInit(KEY_W2_PORT,
            KEY_W2_PIN,
            GPIO_Mode_IN,
            GPIO_PuPd_DOWN);
    PinInit(KEY_W3_PORT,
            KEY_W3_PIN,
            GPIO_Mode_IN,
            GPIO_PuPd_DOWN);
    PinInit(KEY_W4_PORT,
            KEY_W4_PIN,
            GPIO_Mode_IN,
            GPIO_PuPd_DOWN);
    PinInit(KEY_W5_PORT,
            KEY_W5_PIN,
            GPIO_Mode_IN,
            GPIO_PuPd_DOWN);

    clock_ok = ClockInit();
    SystemCoreClockUpdate();

#if BOARD_HAS_BUZZER
    BuzzerInit();
#endif

    (void)SysTick_Config(SystemCoreClock / 1000UL);
    return clock_ok;
}

/**
 * @brief 按位掩码设置 LED。
 * @param mask LED 位掩码。
 */
void Board_Leds(uint8_t mask)
{
#if BOARD_HAS_LEDS
    GPIO_WriteBit(LED_RED_PORT,
                  LED_RED_PIN,
                  (mask & LED_MASK_RED) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LED_GREEN_PORT,
                  LED_GREEN_PIN,
                  (mask & LED_MASK_GREEN) ? Bit_SET : Bit_RESET);
#if BOARD_HAS_WHITE
    GPIO_WriteBit(LED_WHITE_PORT,
                  LED_WHITE_PIN,
                  (mask & LED_MASK_WHITE) ? Bit_SET : Bit_RESET);
#endif
    GPIO_WriteBit(LED_BLUE_PORT,
                  LED_BLUE_PIN,
                  (mask & LED_MASK_BLUE) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LED_YELLOW_PORT,
                  LED_YELLOW_PIN,
                  (mask & LED_MASK_YELLOW) ? Bit_SET : Bit_RESET);
#else
    (void)mask;
#endif
}

/**
 * @brief 读取当前所有按键的原始 GPIO 电平。
 * @return 按键原始电平位掩码。
 */
uint8_t Board_Keys(void)
{
    uint8_t result;

    result = 0U;

    if (GPIO_ReadInputDataBit(KEY_SWITCH_PORT, KEY_SWITCH_PIN))
    {
        result |= 1U;
    }

#if BOARD_HAS_USER
    if (GPIO_ReadInputDataBit(KEY_USER_PORT, KEY_USER_PIN))
    {
        result |= 2U;
    }
#endif

#if BOARD_HAS_CODE
    if (GPIO_ReadInputDataBit(KEY_CODE_PORT, KEY_CODE_PIN))
    {
        result |= 4U;
    }
#endif

    if (GPIO_ReadInputDataBit(KEY_W1_PORT, KEY_W1_PIN))
    {
        result |= 8U;
    }

    if (GPIO_ReadInputDataBit(KEY_W2_PORT, KEY_W2_PIN))
    {
        result |= 16U;
    }

    if (GPIO_ReadInputDataBit(KEY_W3_PORT, KEY_W3_PIN))
    {
        result |= 32U;
    }

    if (GPIO_ReadInputDataBit(KEY_W4_PORT, KEY_W4_PIN))
    {
        result |= 64U;
    }

    if (GPIO_ReadInputDataBit(KEY_W5_PORT, KEY_W5_PIN))
    {
        result |= 128U;
    }

    return result;
}

/**
 * @brief 读取 MCU 唯一标识并生成设备 ID。
 * @return 当前设备 ID。
 */
uint32_t Board_DeviceId(void)
{
    const volatile uint32_t *uid;

    uid = (const volatile uint32_t *)0x1FF80050UL;
    return uid[0] ^ uid[1] ^ uid[2];
}

/**
 * @brief 控制无线芯片 SPI 片选。
 * @param high 置 1 释放片选，置 0 选中芯片。
 */
void Board_Nsel(uint8_t high)
{
    GPIO_WriteBit(GPIOA,
                  RADIO_NSEL_PIN,
                  high ? Bit_SET : Bit_RESET);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

/**
 * @brief 控制无线芯片关断引脚。
 * @param high 置 1 进入关断，置 0 正常工作。
 */
void Board_Sdn(uint8_t high)
{
    GPIO_WriteBit(GPIOA,
                  RADIO_SDN_PIN,
                  high ? Bit_SET : Bit_RESET);
}

/**
 * @brief 配置 SPI1 速度和通信格式。
 * @param fast 是否使用高速配置。
 */
void Board_RadioSpeed(uint8_t fast)
{
    SPI_InitTypeDef spi;

    Board_Nsel(1U);
    SPI_Cmd(RADIO_SPI, DISABLE);
    SPI_I2S_DeInit(RADIO_SPI);
    SPI_StructInit(&spi);
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler =
        fast ? SPI_BaudRatePrescaler_32 :
               SPI_BaudRatePrescaler_128;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(RADIO_SPI, &spi);
    SPI_NSSInternalSoftwareConfig(RADIO_SPI,
                                  SPI_NSSInternalSoft_Set);
    SPI_Cmd(RADIO_SPI, ENABLE);
}

/**
 * @brief 初始化无线 GPIO、SPI1 和 SPI DMA。
 */
void Board_RadioInit(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA |
                          RCC_AHBPeriph_GPIOB,
                          ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    Board_Nsel(1U);
    Board_Sdn(1U);
    PinInit(GPIOA,
            RADIO_NSEL_PIN | RADIO_SDN_PIN,
            GPIO_Mode_OUT,
            GPIO_PuPd_NOPULL);
    PinInit(GPIOA,
            RADIO_SCLK_PIN | RADIO_MOSI_PIN,
            GPIO_Mode_AF,
            GPIO_PuPd_NOPULL);
    PinInit(GPIOA,
            RADIO_MISO_PIN,
            GPIO_Mode_AF,
            GPIO_PuPd_DOWN);
    GPIO_PinAFConfig(GPIOA,
                     GPIO_PinSource5,
                     GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA,
                     GPIO_PinSource6,
                     GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA,
                     GPIO_PinSource7,
                     GPIO_AF_SPI1);
    Board_RadioSpeed(0U);
    SpiDma_Init();
}

/**
 * @brief 等待指定 SPI 标志达到目标状态。
 * @param flag 要等待的 SPI 标志。
 * @param desired 目标状态。
 * @return 达到目标状态时返回 1，超时时返回 0。
 */
static uint8_t WaitSpi(uint16_t flag, FlagStatus desired)
{
    uint32_t start;

    start = Board_Millis();

    while (SPI_I2S_GetFlagStatus(RADIO_SPI, flag) != desired)
    {
        if ((uint32_t)(Board_Millis() - start) >= 5U)
        {
            return 0U;
        }
    }

    return 1U;
}

/**
 * @brief 轮询发送和接收一个 SPI 字节。
 * @param tx 待发送字节。
 * @param rx 接收字节输出地址。
 * @return 传输成功时返回 1，否则返回 0。
 */
uint8_t Board_SpiByte(uint8_t tx, uint8_t *rx)
{
    if (!WaitSpi(SPI_I2S_FLAG_TXE, SET))
    {
        return 0U;
    }

    SPI_I2S_SendData(RADIO_SPI, tx);

    if (!WaitSpi(SPI_I2S_FLAG_RXNE, SET))
    {
        return 0U;
    }

    *rx = (uint8_t)SPI_I2S_ReceiveData(RADIO_SPI);
    return 1U;
}

/**
 * @brief 等待 SPI 忙标志清除。
 * @return SPI 已完成时返回 1，否则返回 0。
 */
uint8_t Board_SpiFinish(void)
{
    return WaitSpi(SPI_I2S_FLAG_BSY, RESET);
}
