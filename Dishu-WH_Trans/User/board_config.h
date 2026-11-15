#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32l1xx.h"

/* Radio bus. */
#define SI4438_SPI                  SPI1
#define SPI_TIMEOUT                 100U
#define SPI_ERROR                   0x01U

/* Interrupt routing. */
#define KEY_SW_IRQn                 EXTI0_IRQn
#define EXTILINE_KEY_SW             EXTI_Line0
#define KEY_AUX_IRQn                EXTI1_IRQn
#define EXTILINE_KEY_AUX            EXTI_Line1
#define KEY_USER_IRQn               EXTI2_IRQn
#define EXTILINE_KEY_USER           EXTI_Line2
#define SI4438_PKGRECV_IRQn         EXTI3_IRQn
#define EXTILINE_SI4438_PKGRECV     EXTI_Line3
#define SI4438_EXITRX_IRQn          EXTI4_IRQn
#define EXTILINE_SI4438_EXITRX      EXTI_Line4

/* Persistent 32-bit device identity. */
#define EEPROM_DEVICE_UID_ADDR      0x08080000U
#define EEPROM_DEVICE_UID_MAGIC_ADDR 0x08080004U
#define DEVICE_UID_MAGIC             0x5AA5A55AUL

/* Power, input, and output pins. */
#define BAT_MEA                     GPIO_Pin_12
#define BP_PWM                      GPIO_Pin_0
#define MOTOR_DRIVER                GPIO_Pin_11
#define LED_LUM_CTR                 GPIO_Pin_14

#define KEY_SW                      GPIO_Pin_0
#define KEY_AUX                     GPIO_Pin_1
#define KEY_USER                    GPIO_Pin_2

#define LED_R_CTR                   GPIO_Pin_3
#define LED_B_CTR                   GPIO_Pin_11
#define LED_G_CTR                   GPIO_Pin_12
#define LED_Y_CTR                   GPIO_Pin_10
#define LED_W_CTR                   GPIO_Pin_9

#define LED_W1_CTR                  GPIO_Pin_9
#define LED_W2_CTR                  GPIO_Pin_6
#define LED_W3_CTR                  GPIO_Pin_8
#define LED_W4_CTR                  GPIO_Pin_5
#define LED_W5_CTR                  GPIO_Pin_4

#define BEEP_VL0                    GPIO_Pin_15
#define BEEP_VL1                    GPIO_Pin_14
#define BEEP_VL2                    GPIO_Pin_13

/* Si4438 pins. */
#define SI_SCLK                     GPIO_Pin_5
#define SI_MISO                     GPIO_Pin_6
#define SI_MOSI                     GPIO_Pin_7
#define SI_NSS                      GPIO_Pin_2
#define SI_SDN                      GPIO_Pin_1
#define SI_GPIO1                    GPIO_Pin_10
#define SI_GPIO2                    GPIO_Pin_4
#define SI_GPIO3                    GPIO_Pin_3

#define PIN_PB7                     GPIO_Pin_7
#define ST_SWDIO                    GPIO_Pin_13
#define ST_SWCLK                    GPIO_Pin_14

/* Buzzer and motor outputs. */
#define BP_PWM_0                    GPIO_ResetBits(GPIOB, BP_PWM)
#define BP_PWM_1                    GPIO_SetBits(GPIOB, BP_PWM)
#define MOTOR_DRIVER_ON             GPIO_ResetBits(GPIOB, MOTOR_DRIVER)
#define MOTOR_DRIVER_OFF            GPIO_SetBits(GPIOB, MOTOR_DRIVER)

/* Illumination output. */
#define LED_LUM_ON                  GPIO_ResetBits(GPIOC, LED_LUM_CTR)
#define LED_LUM_OFF                 GPIO_SetBits(GPIOC, LED_LUM_CTR)

/* Code LEDs. */
#define LED_R_OFF                   GPIO_ResetBits(GPIOB, LED_R_CTR)
#define LED_R_ON                    GPIO_SetBits(GPIOB, LED_R_CTR)
#define LED_B_OFF                   GPIO_ResetBits(GPIOA, LED_B_CTR)
#define LED_B_ON                    GPIO_SetBits(GPIOA, LED_B_CTR)
#define LED_G_OFF                   GPIO_ResetBits(GPIOA, LED_G_CTR)
#define LED_G_ON                    GPIO_SetBits(GPIOA, LED_G_CTR)
#define LED_Y_OFF                   GPIO_ResetBits(GPIOA, LED_Y_CTR)
#define LED_Y_ON                    GPIO_SetBits(GPIOA, LED_Y_CTR)
#define LED_W_OFF                   GPIO_ResetBits(GPIOA, LED_W_CTR)
#define LED_W_ON                    GPIO_SetBits(GPIOA, LED_W_CTR)

/* Position LEDs. */
#define LED_W1_OFF                  GPIO_ResetBits(GPIOB, LED_W1_CTR)
#define LED_W1_ON                   GPIO_SetBits(GPIOB, LED_W1_CTR)
#define LED_W2_OFF                  GPIO_ResetBits(GPIOB, LED_W2_CTR)
#define LED_W2_ON                   GPIO_SetBits(GPIOB, LED_W2_CTR)
#define LED_W3_OFF                  GPIO_ResetBits(GPIOB, LED_W3_CTR)
#define LED_W3_ON                   GPIO_SetBits(GPIOB, LED_W3_CTR)
#define LED_W4_OFF                  GPIO_ResetBits(GPIOB, LED_W4_CTR)
#define LED_W4_ON                   GPIO_SetBits(GPIOB, LED_W4_CTR)
#define LED_W5_OFF                  GPIO_ResetBits(GPIOB, LED_W5_CTR)
#define LED_W5_ON                   GPIO_SetBits(GPIOB, LED_W5_CTR)

/* Volume selectors. */
#define BEEP_VL0_L                  GPIO_ResetBits(GPIOB, BEEP_VL0)
#define BEEP_VL0_H                  GPIO_SetBits(GPIOB, BEEP_VL0)
#define BEEP_VL1_L                  GPIO_ResetBits(GPIOB, BEEP_VL1)
#define BEEP_VL1_H                  GPIO_SetBits(GPIOB, BEEP_VL1)
#define BEEP_VL2_L                  GPIO_ResetBits(GPIOB, BEEP_VL2)
#define BEEP_VL2_H                  GPIO_SetBits(GPIOB, BEEP_VL2)

/* Si4438 GPIO operations. */
#define SI_NSS_0                    GPIO_ResetBits(GPIOA, SI_NSS)
#define SI_NSS_1                    GPIO_SetBits(GPIOA, SI_NSS)
#define SI_MOSI_0                   GPIO_ResetBits(GPIOA, SI_MOSI)
#define SI_MOSI_1                   GPIO_SetBits(GPIOA, SI_MOSI)
#define SI_SCLK_0                   GPIO_ResetBits(GPIOA, SI_SCLK)
#define SI_SCLK_1                   GPIO_SetBits(GPIOA, SI_SCLK)
#define SI_ENABLE                   GPIO_ResetBits(GPIOA, SI_SDN)
#define SI_SHUTDWN                  GPIO_SetBits(GPIOA, SI_SDN)
#define SI_GPIO1_0                  GPIO_ResetBits(GPIOB, SI_GPIO1)
#define SI_GPIO1_1                  GPIO_SetBits(GPIOB, SI_GPIO1)
#define SI_GPIO2_0                  GPIO_ResetBits(GPIOA, SI_GPIO2)
#define SI_GPIO2_1                  GPIO_SetBits(GPIOA, SI_GPIO2)
#define SI_GPIO3_0                  GPIO_ResetBits(GPIOA, SI_GPIO3)
#define SI_GPIO3_1                  GPIO_SetBits(GPIOA, SI_GPIO3)

/* Input status. */
#define KEY_SW_STATUS               GPIO_ReadInputDataBit(GPIOA, KEY_SW)
#define KEY_AUX_STATUS              GPIO_ReadInputDataBit(GPIOB, KEY_AUX)
#define KEY_USER_STATUS             GPIO_ReadInputDataBit(GPIOB, KEY_USER)
#define SI_GPIO1_STATUS             GPIO_ReadInputDataBit(GPIOB, SI_GPIO1)
#define SI_GPIO2_STATUS             GPIO_ReadInputDataBit(GPIOA, SI_GPIO2)
#define SI_GPIO3_STATUS             GPIO_ReadInputDataBit(GPIOA, SI_GPIO3)

#define BUZZER_ON                   Beep_Switch(ENABLE)
#define BUZZER_OFF                  Beep_Switch(DISABLE)
#define BUZZER_VO_LV0               BEEP_VL0_H; BEEP_VL1_L; BEEP_VL2_L
#define BUZZER_VO_LV1               BEEP_VL0_L; BEEP_VL1_H; BEEP_VL2_L
#define BUZZER_VO_LV2               BEEP_VL0_L; BEEP_VL1_L; BEEP_VL2_H
#define BUZZER_VO_LV3               BEEP_VL0_L; BEEP_VL1_L; BEEP_VL2_L

#endif /* BOARD_CONFIG_H */
