#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32l1xx.h"

/* SPI and sensor buses. */
#define SI4438_SPI                  SPI1
#define SPI_TIMEOUT                 100U
#define SPI_ERROR                   0x01U

#define I2C_TIMEOUT                 100U
#define I2C_ERROR                   ERROR
#define I2C_SUCCESS                 SUCCESS
#define I2C_SPEED_LOW               100000U
#define I2C_SPEED_FAST              400000U

/* Interrupt routing. */
#define SWITCH_KEY_IRQn             EXTI0_IRQn
#define EXTI_SWITCH_KEY             EXTI_Line0
#define SI_GPIO1_IRQn               EXTI2_IRQn
#define EXTI_SI_GPIO1               EXTI_Line2

/* Persistent list of paired 32-bit transmitter UIDs. */
#define EEPROM_UID_COUNT_ADDR       0x08080000U
#define EEPROM_UID_START_ADDR       0x08080001U
#define EEPROM_UID_MAGIC_ADDR       0x08080100U
#define PAIRED_UID_MAGIC            0x5AA5A55AUL

/* Debug and board-level pins. */
#define ST_SWDIO                    GPIO_Pin_13
#define ST_SWCLK                    GPIO_Pin_14
#define BAT_MEA                     GPIO_Pin_12

#define KEY_SW                      GPIO_Pin_0
#define KEY_USER                    GPIO_Pin_3
#define LED_G_CTR                   GPIO_Pin_5
#define LED_R_CTR                   GPIO_Pin_6
#define BP_PWM                      GPIO_Pin_8

/* Si4438 pins. */
#define SI_SCLK                     GPIO_Pin_5
#define SI_MISO                     GPIO_Pin_6
#define SI_MOSI                     GPIO_Pin_7
#define SI_NSS                      GPIO_Pin_4
#define SI_SDN                      GPIO_Pin_1
#define SI_GPIO1                    GPIO_Pin_2
#define SI_GPIO3                    GPIO_Pin_2

/* Si4438 GPIO operations. */
#define SI_NSS_0                    GPIO_ResetBits(GPIOA, SI_NSS)
#define SI_NSS_1                    GPIO_SetBits(GPIOA, SI_NSS)
#define SI_MOSI_0                   GPIO_ResetBits(GPIOA, SI_MOSI)
#define SI_MOSI_1                   GPIO_SetBits(GPIOA, SI_MOSI)
#define SI_SCLK_0                   GPIO_ResetBits(GPIOA, SI_SCLK)
#define SI_SCLK_1                   GPIO_SetBits(GPIOA, SI_SCLK)
#define SI_ENABLE                   GPIO_ResetBits(GPIOA, SI_SDN)
#define SI_SHUTDWN                  GPIO_SetBits(GPIOA, SI_SDN)

#define SI_GPIO1_STATUS             GPIO_ReadInputDataBit(GPIOB, SI_GPIO1)
#define SI_GPIO3_STATUS             GPIO_ReadInputDataBit(GPIOA, SI_GPIO3)

/* Keys, LEDs, and buzzer. */
#define KEY_SW_STATUS               GPIO_ReadInputDataBit(GPIOA, KEY_SW)
#define KEY_USER_STATUS             GPIO_ReadInputDataBit(GPIOA, KEY_USER)

#define LED_G_OFF                   GPIO_ResetBits(GPIOB, LED_G_CTR)
#define LED_G_ON                    GPIO_SetBits(GPIOB, LED_G_CTR)
#define LED_R_OFF                   GPIO_ResetBits(GPIOB, LED_R_CTR)
#define LED_R_ON                    GPIO_SetBits(GPIOB, LED_R_CTR)

#define BP_PWM_0                    GPIO_ResetBits(GPIOB, BP_PWM)
#define BP_PWM_1                    GPIO_SetBits(GPIOB, BP_PWM)
#define BP_ON                       TIM_Cmd(TIM4, ENABLE)
#define BP_OFF                      TIM_Cmd(TIM4, DISABLE)

#endif /* BOARD_CONFIG_H */
