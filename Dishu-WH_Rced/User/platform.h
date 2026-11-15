#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include "stm32l1xx.h"

void RCC_Config(void);
void RCC_FreqTest(uint8_t source, uint8_t divider);

void GPIOConfig_ActiveMode(void);
void GPIOConfig_PWROFF_StopMode(void);
void GPIOConfig_PWRON_StopMode(void);
void GPIOConfig_PWRON_ExitFromStopMode(void);

void NVIC_Config(uint8_t irq_channel, uint8_t state);
void EXTI_Config(void);
void Enter_StopMode(void);
void Enter_StandbyMode(void);

void TIM6_Delay_Config(void);
void Delay_TIM6_1ms(uint32_t time);
void TIM4_Beep_PWMConfig(void);
void TIM7_Irq_Config(uint8_t time);
void TIM7_SiTransIrq_Config(uint16_t time);

void ADC_Config(uint8_t channel);
uint16_t Battery_Meas(void);

void SPI_Config(SPI_TypeDef *spi);
uint8_t SPI_ByteWR(SPI_TypeDef *spi, uint8_t data);

void UART_Config(USART_TypeDef *uart);
void UART_ByteSend(USART_TypeDef *uart, uint8_t data);
void UART_StringSend(USART_TypeDef *uart, uint8_t length, uint8_t *data);
uint8_t UART_ByteRead(USART_TypeDef *uart);
void UART_StringRead(USART_TypeDef *uart, uint8_t length, uint8_t *data);

ErrorStatus I2C_Config(I2C_TypeDef *i2c);
void I2C_ByteSend(I2C_TypeDef *i2c, uint8_t addr, uint8_t data);
void I2C_StringSend(I2C_TypeDef *i2c, uint8_t addr, uint8_t length,
                    uint8_t *data);
uint8_t I2C_Byte_Read(I2C_TypeDef *i2c, uint8_t addr);
void I2C_Read(I2C_TypeDef *i2c, uint8_t addr, uint8_t length,
              uint8_t *data);

void EEPROM_ByteWR(uint32_t addr, uint8_t data);
void EEPROM_StringWR(uint32_t addr, uint8_t length, uint8_t *data);
uint8_t EEPROM_ByteRD(uint32_t addr);
void EEPROM_StringRD(uint32_t addr, uint8_t length, uint8_t *data);

void Delay_General(uint32_t delay);
void RTC_LSIWUConfig(uint8_t wakeup_interval);
void RTC_LSIDisableConfig(void);

#endif /* PLATFORM_H */
