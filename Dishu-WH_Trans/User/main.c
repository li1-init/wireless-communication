#include "stm32l1xx.h"
#include "board_config.h"
#include "platform.h"
#include "app_state.h"
#include "app_protocol.h"
#include "app_radio.h"
#include "device_uid.h"
#include "led.h"

#define ACK_RETRY_COUNT  3U

static uint8_t TransmitUntilAck(uint32_t uid, uint8_t command)
{
    uint8_t sequence;
    uint8_t attempt;

    sequence = AppProtocol_NextSequence();
    AppProtocol_BuildRequest(Trans_Send, command, uid, sequence);

    for (attempt = 0U; attempt < ACK_RETRY_COUNT; ++attempt)
    {
        AppState_ClearRadioPacketReady();
        tim7_irq_flag = 0U;

        LED_Set(LED_BLUE, LED_STATE_ON);
        AppRadio_SendPacket(Trans_Send);
        LED_Set(LED_BLUE, LED_STATE_OFF);

        NVIC_Config(SI4438_EXITRX_IRQn, ENABLE);
        AppRadio_StartReceive();
        TIM7_Irq_Config(1U);
        NVIC_Config(TIM7_IRQn, ENABLE);

        while ((!AppState_IsRadioPacketReady()) && (!tim7_irq_flag))
        {
        }

        NVIC_Config(TIM7_IRQn, DISABLE);
        AppRadio_EnterSleep();

        if ((AppState_IsRadioPacketReady() != 0U) &&
            (AppProtocol_IsAckFor(AppRadio_GetReceivedPacket(),
                                  uid,
                                  sequence) != 0U))
        {
            LED_Blink(LED_BLUE, 3U, 100U);
            return 1U;
        }
    }

    AppState_ClearRadioPacketReady();
    return 0U;
}

int main(void)
{
    RCC_Config();
    PWR_WakeUpPinCmd(PWR_WakeUpPin_1, DISABLE);
    TIM6_Delay_Config();

    AppState_Init();
    GPIOConfig_ActiveMode();
    LED_Init();
    EXTI_NVIC_Config();

    AppRadio_Init();
    (void)DeviceUID_Get();

    while (1)
    {
        AppRadio_EnterSleep();

        NVIC_Config(KEY_SW_IRQn, ENABLE);
        NVIC_Config(KEY_USER_IRQn, ENABLE);
        NVIC_Config(KEY_AUX_IRQn, ENABLE);
        NVIC_Config(TIM7_IRQn, DISABLE);

        GPIOConfig_PWRON_StopMode();
        Enter_StopMode();

        RCC_Config();
        GPIOConfig_PWRON_ExitFromStopMode();
        NVIC_Config(SI4438_EXITRX_IRQn, ENABLE);

        while (!AppState_HasButtonEvent())
        {
        }

        (void)TransmitUntilAck(DeviceUID_Get(),
                               AppProtocol_GetTxCommand());
        Delay_TIM6_1ms(300U);
    }
}
