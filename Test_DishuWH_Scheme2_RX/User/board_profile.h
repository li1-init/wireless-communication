/**
 * @file board_profile.h
 * @brief 当前方形接收板的硬件能力和引脚定义。
 */
#ifndef BOARD_PROFILE_H
#define BOARD_PROFILE_H
/* Red=PB3, green=PA11, white=PA9, blue=PA12, yellow=PA10.
 * Keys are PA0, PB2, PB1 and PB4-PB9. */
#define BOARD_IS_TX 0
#define BOARD_HAS_LEDS 1
#define BOARD_HAS_BUZZER 0
#define BOARD_HAS_WHITE 1
#define BOARD_HAS_CODE 1
#define BOARD_HAS_USER 1
#define RADIO_NSEL_PIN GPIO_Pin_2
#define LED_RED_PORT GPIOB
#define LED_RED_PIN GPIO_Pin_3
#define LED_GREEN_PORT GPIOA
#define LED_GREEN_PIN GPIO_Pin_11
#define LED_WHITE_PORT GPIOA
#define LED_WHITE_PIN GPIO_Pin_9
#define LED_BLUE_PORT GPIOA
#define LED_BLUE_PIN GPIO_Pin_12
#define LED_YELLOW_PORT GPIOA
#define LED_YELLOW_PIN GPIO_Pin_10
#define KEY_SWITCH_PORT GPIOA
#define KEY_SWITCH_PIN GPIO_Pin_0
#define KEY_USER_PORT GPIOB
#define KEY_USER_PIN GPIO_Pin_2
#define KEY_CODE_PORT GPIOB
#define KEY_CODE_PIN GPIO_Pin_1
/* W1-W5 are high-active button inputs with internal pull-down. */
#define KEY_W1_PORT GPIOB
#define KEY_W1_PIN GPIO_Pin_9
#define KEY_W2_PORT GPIOB
#define KEY_W2_PIN GPIO_Pin_6
#define KEY_W3_PORT GPIOB
#define KEY_W3_PIN GPIO_Pin_8
#define KEY_W4_PORT GPIOB
#define KEY_W4_PIN GPIO_Pin_5
#define KEY_W5_PORT GPIOB
#define KEY_W5_PIN GPIO_Pin_4
#endif
