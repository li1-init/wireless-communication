/* 文件说明：板型引脚表：指定当前板型实际使用的无线、按键及提示器件引脚。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef BOARD_PROFILE_H
#define BOARD_PROFILE_H
/* Rectangular transmitter / 发射.pcb; measured by user 2026-11-16. */
#define BOARD_IS_TX 1
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
/* W1-W5 are LED/trigger nets: internal pull-down, active at 3.3 V. */
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
