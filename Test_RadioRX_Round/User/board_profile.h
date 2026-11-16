#ifndef BOARD_PROFILE_H
#define BOARD_PROFILE_H
/* Round receiver / 接收.pcb. User confirmed NO LEDs fitted, only rear buzzer.
 * PB8 (U2.45) -> Q1/Q2 -> C8 -> B1. PWM is TIM4_CH3, AF2.
 * Radio NSEL is PA4, NOT transmitter PA2. PB5/PB6 are left untouched. */
#define BOARD_IS_TX 0
#define BOARD_HAS_LEDS 0
#define BOARD_HAS_BUZZER 0
#define BOARD_HAS_WHITE 0
#define BOARD_HAS_CODE 0
#define RADIO_NSEL_PIN GPIO_Pin_4
#define BUZZER_PORT GPIOB
#define BUZZER_PIN GPIO_Pin_8
#define BUZZER_PIN_SOURCE GPIO_PinSource8
#define KEY_SWITCH_PORT GPIOA
#define KEY_SWITCH_PIN GPIO_Pin_0
#define KEY_USER_PORT GPIOA
#define KEY_USER_PIN GPIO_Pin_3
#endif
