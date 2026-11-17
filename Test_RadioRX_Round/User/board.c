/* 文件说明：板级驱动：初始化时钟、GPIO、SPI、按键以及灯或蜂鸣器。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#include "board.h"
#include "spidma.h"

uint32_t SystemCoreClock = 2097152UL;
static volatile uint32_t ticks;

/* Invoked before C runtime initialization. Keep reset MSI clock until main.
 * Do not use the legacy HSE/PLL startup: board documents disagree on HSE. */
/** @brief 复位早期设置中断向量表；此时不能依赖尚未初始化的全局变量。 */
void SystemInit(void)
{
    SCB->VTOR = FLASH_BASE;
}

/** @brief 根据 RCC 当前配置更新系统时钟频率变量。 */
void SystemCoreClockUpdate(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    SystemCoreClock = clocks.HCLK_Frequency;
}

/** @brief 每毫秒递增时基，并推进灯或蜂鸣器的计时。 */
void SysTick_Handler(void)
{
    ++ticks;
#if BOARD_HAS_BUZZER
    Sound_Tick1ms();
#endif
}
/** @brief 返回毫秒时基；计算时间差使用无符号减法以兼容回绕。 */
uint32_t Board_Millis(void) { return ticks; }
/** @brief 等待指定毫秒数；中断仍可执行，但主循环暂停。 */
void Board_DelayMs(uint32_t ms)
{
    uint32_t start = ticks;
    while ((uint32_t)(ticks - start) < ms) { }
}

/** @brief 统一配置一个 GPIO 端口上的指定引脚。 */
static void PinInit(GPIO_TypeDef *port, uint16_t pins, GPIOMode_TypeDef mode,
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

/** @brief 配置内部 HSI 和总线时钟，带超时检查；成功返回 1。 */
static uint8_t ClockInit(void)
{
    uint32_t guard;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_VoltageScalingConfig(PWR_VoltageScaling_Range1);
    for (guard = 0; guard < 200000UL; ++guard)
        if (PWR_GetFlagStatus(PWR_FLAG_VOS) == RESET) break;
    if (guard == 200000UL) return 0U;

    FLASH_SetLatency(FLASH_Latency_1);
    RCC_HSICmd(ENABLE);
    for (guard = 0; guard < 200000UL; ++guard)
        if (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == SET) break;
    if (guard == 200000UL) return 0U;

    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    for (guard = 0; guard < 200000UL; ++guard)
        if (RCC_GetSYSCLKSource() == 0x04U) break;
    if (guard == 200000UL) return 0U;
    SystemCoreClockUpdate();
    return SystemCoreClock == 16000000UL ? 1U : 0U;
}

#if BOARD_HAS_BUZZER
/** @brief 初始化 TIM4_CH3 的蜂鸣器 PWM，初始输出关闭。 */
static void BuzzerInit(void)
{
    TIM_TimeBaseInitTypeDef timer;
    TIM_OCInitTypeDef channel;
    RCC_ClocksTypeDef clocks;
    uint32_t timer_hz;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
    GPIO_ResetBits(BUZZER_PORT,BUZZER_PIN);
    PinInit(BUZZER_PORT,BUZZER_PIN,GPIO_Mode_OUT,GPIO_PuPd_NOPULL);
    RCC_GetClocksFreq(&clocks);
    timer_hz=clocks.PCLK1_Frequency;
    if(clocks.PCLK1_Frequency!=clocks.HCLK_Frequency) timer_hz*=2U;
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler=(uint16_t)(timer_hz/1000000UL-1UL);
    timer.TIM_Period=343U;
    TIM_TimeBaseInit(TIM4,&timer);
    TIM_OCStructInit(&channel);
    channel.TIM_OCMode=TIM_OCMode_PWM1;
    channel.TIM_OutputState=TIM_OutputState_Enable;
    channel.TIM_OCPolarity=TIM_OCPolarity_High;
    channel.TIM_Pulse=0U;
    TIM_OC3Init(TIM4,&channel);
    TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4,ENABLE);
    TIM_GenerateEvent(TIM4,TIM_EventSource_Update);
    GPIO_PinAFConfig(BUZZER_PORT,BUZZER_PIN_SOURCE,GPIO_AF_TIM4);
    PinInit(BUZZER_PORT,BUZZER_PIN,GPIO_Mode_AF,GPIO_PuPd_NOPULL);
    /* 16MHz / 16 / 344 ~= 2907Hz, 50% duty. Matches legacy RX tone,
     * adapted from its 32MHz clock. CCR3=0 is silent and output stays low. */
    TIM_Cmd(TIM4,ENABLE);
}
#endif
/** @brief 直接控制蜂鸣器 PWM 占空比，实现发声或静音。 */
void Board_SoundGate(uint8_t on)
{
#if BOARD_HAS_BUZZER
    TIM4->CCR3=on ? 172U : 0U;
#else
    (void)on;
#endif
}
/** @brief 在短临界区内启动提示音，避免与 SysTick 同时修改声音状态。 */
void Board_Sound(uint8_t event,uint8_t code)
{
#if BOARD_HAS_BUZZER
    uint32_t saved=__get_PRIMASK();
    __disable_irq();
    Sound_Play(event,code);
    __set_PRIMASK(saved);
#else
    (void)event;(void)code;
#endif
}
/** @brief 初始化当前板型的时钟、按键、提示器件和毫秒时基。 */
uint8_t Board_Init(void)
{
    uint8_t clock_ok;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);
#if BOARD_HAS_LEDS
    Board_Leds(0U);
    PinInit(LED_RED_PORT, LED_RED_PIN, GPIO_Mode_OUT, GPIO_PuPd_NOPULL);
    PinInit(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_Mode_OUT, GPIO_PuPd_NOPULL);
#if BOARD_HAS_WHITE
    PinInit(LED_WHITE_PORT, LED_WHITE_PIN, GPIO_Mode_OUT, GPIO_PuPd_NOPULL);
#endif
#endif
    PinInit(KEY_SWITCH_PORT, KEY_SWITCH_PIN, GPIO_Mode_IN, GPIO_PuPd_NOPULL);
#if BOARD_HAS_USER
    PinInit(KEY_USER_PORT, KEY_USER_PIN, GPIO_Mode_IN, GPIO_PuPd_NOPULL);
#endif
#if BOARD_HAS_CODE
    PinInit(KEY_CODE_PORT, KEY_CODE_PIN, GPIO_Mode_IN, GPIO_PuPd_NOPULL);
#endif
    /* PA13/PA14 remain SWD. Only this board profile's fitted indicators run. */
    clock_ok = ClockInit();
    SystemCoreClockUpdate();
#if BOARD_HAS_BUZZER
    BuzzerInit();
#endif
    (void)SysTick_Config(SystemCoreClock / 1000UL);
    return clock_ok;
}

/** @brief 按位掩码设置方板 LED；未装灯的板型不执行硬件输出。 */
void Board_Leds(uint8_t mask)
{
#if BOARD_HAS_LEDS
    GPIO_WriteBit(LED_RED_PORT, LED_RED_PIN, (mask & LED_MASK_RED) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LED_GREEN_PORT, LED_GREEN_PIN, (mask & LED_MASK_GREEN) ? Bit_SET : Bit_RESET);
#if BOARD_HAS_WHITE
    GPIO_WriteBit(LED_WHITE_PORT, LED_WHITE_PIN, (mask & LED_MASK_WHITE) ? Bit_SET : Bit_RESET);
#endif
#else
    (void)mask;
#endif
}

/** @brief 读取实际安装的按键，按 SWITCH/USER/CODE 返回对应位。 */
uint8_t Board_Keys(void)
{
    uint8_t result = 0U;
    if (GPIO_ReadInputDataBit(KEY_SWITCH_PORT, KEY_SWITCH_PIN)) result |= 1U;
#if BOARD_HAS_USER
    if (GPIO_ReadInputDataBit(KEY_USER_PORT, KEY_USER_PIN)) result |= 2U;
#endif
#if BOARD_HAS_CODE
    if (GPIO_ReadInputDataBit(KEY_CODE_PORT, KEY_CODE_PIN)) result |= 4U;
#endif
    return result;
}

/** @brief 读取 MCU 唯一标识并生成当前协议使用的设备 ID。 */
uint32_t Board_DeviceId(void)
{
    /* STM32L1 medium-density 96-bit unique ID. Read only; no EEPROM writes. */
    const volatile uint32_t *uid = (const volatile uint32_t *)0x1FF80050UL;
    return uid[0] ^ uid[1] ^ uid[2];
}

/** @brief 控制无线芯片 SPI 片选，保留短暂建立时间。 */
void Board_Nsel(uint8_t high)
{
    GPIO_WriteBit(GPIOA, RADIO_NSEL_PIN, high ? Bit_SET : Bit_RESET);
    /* Minimum NSEL high/setup time, also at the faster test speed. */
    __NOP(); __NOP(); __NOP(); __NOP();
}
/** @brief 控制无线芯片关断引脚。 */
void Board_Sdn(uint8_t high)
{
    GPIO_WriteBit(GPIOA, RADIO_SDN_PIN, high ? Bit_SET : Bit_RESET);
}

/** @brief 配置 SPI1 时钟、极性和数据格式。 */
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
    spi.SPI_BaudRatePrescaler = fast ? SPI_BaudRatePrescaler_32 : SPI_BaudRatePrescaler_128;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(RADIO_SPI, &spi);
    SPI_NSSInternalSoftwareConfig(RADIO_SPI, SPI_NSSInternalSoft_Set);
    SPI_Cmd(RADIO_SPI, ENABLE);
}

/** @brief 初始化无线 GPIO、SPI1 以及 SPI DMA 通道。 */
void Board_RadioInit(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    Board_Nsel(1U);
    Board_Sdn(1U);
    PinInit(GPIOA, RADIO_NSEL_PIN | RADIO_SDN_PIN, GPIO_Mode_OUT, GPIO_PuPd_NOPULL);
    PinInit(GPIOA, RADIO_SCLK_PIN | RADIO_MOSI_PIN, GPIO_Mode_AF, GPIO_PuPd_NOPULL);
    /* A disconnected MISO should tend to 0, not a false 0xFF CTS. */
    PinInit(GPIOA, RADIO_MISO_PIN, GPIO_Mode_AF, GPIO_PuPd_DOWN);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
    /* Poll over SPI; do not initialize auxiliary GPIOs using another board's
     * map. In particular RX PA4 is NSEL, and RX PB2 is radio GPIO1, not USER. */
    Board_RadioSpeed(0U);
    SpiDma_Init();
}

/** @brief 等待指定 SPI 标志达到目标状态，超时返回失败。 */
static uint8_t WaitSpi(uint16_t flag, FlagStatus desired)
{
    uint32_t start = Board_Millis();
    while (SPI_I2S_GetFlagStatus(RADIO_SPI, flag) != desired)
        if ((uint32_t)(Board_Millis() - start) >= 5U) return 0U;
    return 1U;
}

/** @brief 轮询发送和接收一个 SPI 字节，用于短命令和状态查询。 */
uint8_t Board_SpiByte(uint8_t tx, uint8_t *rx)
{
    if (!WaitSpi(SPI_I2S_FLAG_TXE, SET)) return 0U;
    SPI_I2S_SendData(RADIO_SPI, tx);
    if (!WaitSpi(SPI_I2S_FLAG_RXNE, SET)) return 0U;
    *rx = (uint8_t)SPI_I2S_ReceiveData(RADIO_SPI);
    return 1U;
}

/** @brief 等待 SPI 忙标志清除，确保最后一位真正发出。 */
uint8_t Board_SpiFinish(void)
{
    return WaitSpi(SPI_I2S_FLAG_BSY, RESET);
}
