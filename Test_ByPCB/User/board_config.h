#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32l1xx.h"

/* ============================================================================
 * Test_ByPCB : pin map reconciled against the two hardware tests.
 *   MCU = U1 STM32L151C8T6, LQFP-48.
 *
 *   Working pins confirmed on the physical board:
 *     RED = PB3, GREEN = PA11.
 *   The two source layouts disagree about blue:
 *     PADS netlist: PA12
 *     PDF layout:   PB15
 *   This diagnostic build drives both candidates for the blue channel.
 * ==========================================================================*/

/* ---- Radio bus / interrupts (kept from the original template) ------------ */
#define SI4438_SPI                  SPI1
#define SPI_TIMEOUT                 100U
#define SPI_ERROR                   0x01U

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

#define EEPROM_DEVICE_UID_ADDR      0x08080000U
#define EEPROM_DEVICE_UID_MAGIC_ADDR 0x08080004U
#define DEVICE_UID_MAGIC             0x5AA5A55AUL

/* ---- LED channels (measured physical board) -------------------------------
 * LED_R_CTR = PB3  (pin 39)  <-- JTDO
 * LED_G_CTR = PA11 (pin 32)
 * LED_B_CTR = PB15 (pin 28)
 * LED_B_ALT = PA12 (pin 33, PADS netlist candidate)
 *
 * PA12 was marked BLED_CTR in the PADS file, but it does not drive the blue
 * LED on the tested physical board.  Do not use PA12 for the blue channel.
 */
#define LED_R_PORT              GPIOB
#define LED_R_PIN               GPIO_Pin_3
#define LED_G_PORT              GPIOA
#define LED_G_PIN               GPIO_Pin_11
#define LED_B_PORT              GPIOB
#define LED_B_PIN               GPIO_Pin_15
#define LED_B_ALT_PORT          GPIOA
#define LED_B_ALT_PIN           GPIO_Pin_12
#define LED_Y_PORT              GPIOA
#define LED_Y_PIN               GPIO_Pin_10
#define LED_W_PORT              GPIOA
#define LED_W_PIN               GPIO_Pin_9

/* ---- Keys (PCB) -----------------------------------------------------------
 * SW1 KEY_SW   = PA0 (pin 10)
 * SW2 KEY_USER = PB2 (pin 20)
 * SW3 KEY_CODE = PB1 (pin 19)
 */
#define KEY1_PORT               GPIOA
#define KEY1_PIN                GPIO_Pin_0
#define KEY2_PORT               GPIOB
#define KEY2_PIN                GPIO_Pin_2
#define KEY3_PORT               GPIOB
#define KEY3_PIN                GPIO_Pin_1

/* ---- Si4438 SPI (PCB) -----------------------------------------------------
 * SCLK = PA5 (pin 15), MISO = PA6 (pin 16), MOSI = PA7 (pin 17)
 * nSEL = PA2 (pin 12), SDN = PA1 (pin 11)
 */
#define SI_SCLK_PORT            GPIOA
#define SI_SCLK_PIN             GPIO_Pin_5
#define SI_MISO_PORT            GPIOA
#define SI_MISO_PIN             GPIO_Pin_6
#define SI_MOSI_PORT            GPIOA
#define SI_MOSI_PIN             GPIO_Pin_7
#define SI_NSS_PORT             GPIOA
#define SI_NSS_PIN              GPIO_Pin_2
#define SI_SDN_PORT             GPIOA
#define SI_SDN_PIN              GPIO_Pin_1

/* Aliases used by the original SI4438 driver (must exist for linking). */
#define SI_SCLK                     SI_SCLK_PIN
#define SI_MISO                     SI_MISO_PIN
#define SI_MOSI                     SI_MOSI_PIN
#define SI_NSS                      SI_NSS_PIN
#define SI_SDN                      SI_SDN_PIN
#define SI_GPIO1                    GPIO_Pin_10
#define SI_GPIO2                    GPIO_Pin_4
#define SI_GPIO3                    GPIO_Pin_3

#define SI_NSS_0                    GPIO_ResetBits(SI_NSS_PORT, SI_NSS_PIN)
#define SI_NSS_1                    GPIO_SetBits(SI_NSS_PORT, SI_NSS_PIN)
#define SI_MOSI_0                   GPIO_ResetBits(SI_MOSI_PORT, SI_MOSI_PIN)
#define SI_MOSI_1                   GPIO_SetBits(SI_MOSI_PORT, SI_MOSI_PIN)
#define SI_SCLK_0                   GPIO_ResetBits(SI_SCLK_PORT, SI_SCLK_PIN)
#define SI_SCLK_1                   GPIO_SetBits(SI_SCLK_PORT, SI_SCLK_PIN)
#define SI_ENABLE                   GPIO_ResetBits(SI_SDN_PORT, SI_SDN_PIN)
#define SI_SHUTDWN                  GPIO_SetBits(SI_SDN_PORT, SI_SDN_PIN)
#define SI_GPIO1_STATUS             GPIO_ReadInputDataBit(GPIOB, SI_GPIO1)
#define SI_GPIO2_STATUS             GPIO_ReadInputDataBit(GPIOA, SI_GPIO2)
#define SI_GPIO3_STATUS             GPIO_ReadInputDataBit(GPIOA, SI_GPIO3)

/* Legacy aliases needed by User_STM32Lxx_Peripheral.c / stm32l1xx_it.c */
#define LED_B_CTR                   LED_B_PIN
#define LED_B_OFF                   GPIO_ResetBits(LED_B_PORT, LED_B_PIN)
#define LED_B_ON                    GPIO_SetBits(LED_B_PORT, LED_B_PIN)
#define KEY_SW                      KEY1_PIN
#define KEY_AUX                     KEY3_PIN
#define KEY_USER                    KEY2_PIN
#define KEY_SW_STATUS               GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN)
#define KEY_AUX_STATUS              GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN)
#define KEY_USER_STATUS             GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN)

/* Debug pin aliases (SWD) */
#define ST_SWDIO                    GPIO_Pin_13
#define ST_SWCLK                    GPIO_Pin_14

#endif /* BOARD_CONFIG_H */

