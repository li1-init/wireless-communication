/* 文件说明：板型引脚表：指定当前板型实际使用的无线、按键及提示器件引脚。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef BOARD_PROFILE_H
#define BOARD_PROFILE_H
/* Round receiver / 接收.pcb. User confirmed NO LEDs fitted, only rear buzzer.
 * PB8 (U2.45) -> Q1/Q2 -> C8 -> B1. PWM is TIM4_CH3, AF2.
 * Only SW KEY (PA0) is fitted; PA3 USER KEY is not populated.
 * Radio NSEL is PA4, NOT transmitter PA2. PB5/PB6 are left untouched. */
#define BOARD_IS_TX 0
#define BOARD_HAS_LEDS 0
#define BOARD_HAS_BUZZER 1
#define BOARD_HAS_WHITE 0
#define BOARD_HAS_CODE 0
#define BOARD_HAS_USER 0
#define RADIO_NSEL_PIN GPIO_Pin_4
#define BUZZER_PORT GPIOB
#define BUZZER_PIN GPIO_Pin_8
#define BUZZER_PIN_SOURCE GPIO_PinSource8
#define KEY_SWITCH_PORT GPIOA
#define KEY_SWITCH_PIN GPIO_Pin_0
#endif
