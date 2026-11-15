#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include "stm32l1xx.h"

void RCC_Config(void);
void RCC_FreqTest(uint8_t source, uint8_t divider);

void GPIOConfig_ActiveMode(void);
void GPIOConfig_PWRON_StopMode(void);
void GPIOConfig_PWROFF_StopMode(void);
void GPIOConfig_PWRON_ExitFromStopMode(void);

void NVIC_Config(uint8_t irq_channel, uint8_t state);
void EXTI_NVIC_Config(void);
void Enter_StopMode(void);
void Enter_StandbyMode(void);

void TIM6_Delay_Config(void);
void Delay_TIM6_1ms(uint32_t time);
void TIM11_Delay_Config(void);
void Delay_TIM11_Irq_1ms(uint32_t time);
void TIM3_Beep_PWMConfig(uint16_t tone);
void TIM2_Irq_Config(uint8_t time);
void TIM4_LEDBlink_Config(uint8_t blink_time);
void TIM7_Irq_Config(uint8_t time);

void ADC_Config(uint8_t channel);
uint16_t Battery_Meas(void);

void SPI_Config(SPI_TypeDef *spi);
uint8_t SPI_ByteWR(SPI_TypeDef *spi, uint8_t data);

void EEPROM_ByteWR(uint32_t addr, uint8_t data);
void EEPROM_StringWR(uint32_t addr, uint8_t length, uint8_t *data);
uint8_t EEPROM_ByteRD(uint32_t addr);
void EEPROM_StringRD(uint32_t addr, uint8_t length, uint8_t *data);

void Delay_General(uint32_t delay);
void RTC_LSIWUConfig(uint8_t wakeup_interval);
void RTC_LSIDisableConfig(void);

#endif /* PLATFORM_H */
