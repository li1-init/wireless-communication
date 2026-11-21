/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：方形接收板引脚表，硬件与原来的方板发送板一致。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 根据编译条件选择对应实现。 */
#ifndef BOARD_PROFILE_H
/* 定义编译期常量或接口宏。 */
#define BOARD_PROFILE_H
/* Square receiver: the second original square transmitter board.
 * Red=PB3, green=PA11, white/blue=PA9; keys PA0/PB2/PB1.
 * The square board has no confirmed buzzer, so feedback uses these LEDs. */
/* 定义编译期常量或接口宏。 */
#define BOARD_IS_TX 0
/* 定义编译期常量或接口宏。 */
#define BOARD_HAS_LEDS 1
/* 定义编译期常量或接口宏。 */
#define BOARD_HAS_BUZZER 0
/* 定义编译期常量或接口宏。 */
#define BOARD_HAS_WHITE 1
/* 定义编译期常量或接口宏。 */
#define BOARD_HAS_CODE 1
/* 定义编译期常量或接口宏。 */
#define BOARD_HAS_USER 1
/* 定义编译期常量或接口宏。 */
#define RADIO_NSEL_PIN GPIO_Pin_2
/* 定义编译期常量或接口宏。 */
#define LED_RED_PORT GPIOB
/* 定义编译期常量或接口宏。 */
#define LED_RED_PIN GPIO_Pin_3
/* 定义编译期常量或接口宏。 */
#define LED_GREEN_PORT GPIOA
/* 定义编译期常量或接口宏。 */
#define LED_GREEN_PIN GPIO_Pin_11
/* 定义编译期常量或接口宏。 */
#define LED_WHITE_PORT GPIOA
/* 定义编译期常量或接口宏。 */
#define LED_WHITE_PIN GPIO_Pin_9
/* 定义编译期常量或接口宏。 */
#define LED_BLUE_PORT GPIOA
/* 定义编译期常量或接口宏。 */
#define LED_BLUE_PIN GPIO_Pin_12
/* 定义编译期常量或接口宏。 */
#define LED_YELLOW_PORT GPIOA
/* 定义编译期常量或接口宏。 */
#define LED_YELLOW_PIN GPIO_Pin_10
/* 定义编译期常量或接口宏。 */
#define KEY_SWITCH_PORT GPIOA
/* 定义编译期常量或接口宏。 */
#define KEY_SWITCH_PIN GPIO_Pin_0
/* 定义编译期常量或接口宏。 */
#define KEY_USER_PORT GPIOB
/* 定义编译期常量或接口宏。 */
#define KEY_USER_PIN GPIO_Pin_2
/* 定义编译期常量或接口宏。 */
#define KEY_CODE_PORT GPIOB
/* 定义编译期常量或接口宏。 */
#define KEY_CODE_PIN GPIO_Pin_1
/* W1-W5 are high-active button inputs with internal pull-down. */
/* 定义编译期常量或接口宏。 */
#define KEY_W1_PORT GPIOB
/* 定义编译期常量或接口宏。 */
#define KEY_W1_PIN GPIO_Pin_9
/* 定义编译期常量或接口宏。 */
#define KEY_W2_PORT GPIOB
/* 定义编译期常量或接口宏。 */
#define KEY_W2_PIN GPIO_Pin_6
/* 定义编译期常量或接口宏。 */
#define KEY_W3_PORT GPIOB
/* 定义编译期常量或接口宏。 */
#define KEY_W3_PIN GPIO_Pin_8
/* 定义编译期常量或接口宏。 */
#define KEY_W4_PORT GPIOB
/* 定义编译期常量或接口宏。 */
#define KEY_W4_PIN GPIO_Pin_5
/* 定义编译期常量或接口宏。 */
#define KEY_W5_PORT GPIOB
/* 定义编译期常量或接口宏。 */
#define KEY_W5_PIN GPIO_Pin_4
/* 根据编译条件选择对应实现。 */
#endif
