#include "stm32l1xx.h"
#include "board_config.h"
#include "platform.h"
#include "app_state.h"
#include "app_protocol.h"
#include "app_radio.h"
#include "device_uid.h"
#include "led.h"
#include "buzzer.h"
#include "alarm.h"

int main(void)
{
    RCC_Config();
    PWR_WakeUpPinCmd(PWR_WakeUpPin_1, DISABLE);
    TIM6_Delay_Config();

    AppState_Init();
    Pairing_Init();
    GPIOConfig_ActiveMode();
    LED_Init();
    Buzzer_Init();
    EXTI_Config();

    AppRadio_Init();
    NVIC_Config(SWITCH_KEY_IRQn, ENABLE);

    while (1)
    {
        NVIC_Config(SI_GPIO1_IRQn, ENABLE);
        AppRadio_StartReceive();

        while (!AppState_IsRadioPacketReady())
        {
        }

        if ((AppProtocol_IsValid(AppRadio_GetReceivedPacket()) != 0U) &&
            (AppProtocol_IsSupportedCommand(
                 AppProtocol_GetCommand(AppRadio_GetReceivedPacket())) != 0U))
        {
            uint8_t command =
                AppProtocol_GetCommand(AppRadio_GetReceivedPacket());
            uint32_t uid =
                AppProtocol_GetUID(AppRadio_GetReceivedPacket());
            uint8_t accepted = 0U;

            if (command == ALARM_CODING)
            {
                if ((AppState_IsPairingMode() != 0U) ||
                    (Pairing_IsPaired(uid) != 0U))
                {
                    accepted = Pairing_Add(uid);
                    if (accepted != 0U)
                    {
                        AppState_SetPairingMode(0U);
                        LED_Set(LED_RED, LED_STATE_OFF);
                    }
                }
            }
            else if (Pairing_IsPaired(uid) != 0U)
            {
                accepted = 1U;
            }

            if (accepted != 0U)
            {
                AppProtocol_BuildAck(si4438_txdata,
                                     AppRadio_GetReceivedPacket());
                AppRadio_SendPacket(si4438_txdata);

                EXTI->IMR &= ~EXTI_SWITCH_KEY;
                Alarm_Show(command);
                EXTI->IMR |= EXTI_SWITCH_KEY;
            }
        }
    }
}
