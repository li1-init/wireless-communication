#include "app_state.h"

volatile uint8_t Trans_Send[APP_RADIO_PACKET_SIZE] =
{
    APP_PROTOCOL_MAGIC,
    ALARM_SWITCHOFF,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U
};

volatile uint8_t switchkey_irq_flag = 0U;
volatile uint8_t userkey_irq_flag = 0U;
volatile uint8_t auxkey_irq_flag = 0U;
volatile uint8_t si4438pkgrecv_irq_flag = 0U;
volatile uint8_t si4438_validpkg_flag = 0U;
volatile uint8_t tim2_irq_flag = 0U;
volatile uint8_t tim7_irq_flag = 0U;

volatile uint8_t lum_mode = 0U;
volatile uint8_t power_off_flag = 0U;

void AppState_Init(void)
{
    switchkey_irq_flag = 0U;
    userkey_irq_flag = 0U;
    auxkey_irq_flag = 0U;
    si4438pkgrecv_irq_flag = 0U;
    si4438_validpkg_flag = 0U;
    tim2_irq_flag = 0U;
    tim7_irq_flag = 0U;
}

uint8_t AppState_HasButtonEvent(void)
{
    return (uint8_t)(switchkey_irq_flag |
                     userkey_irq_flag |
                     auxkey_irq_flag);
}

uint8_t AppState_IsRadioPacketReady(void)
{
    return si4438_validpkg_flag;
}

void AppState_ClearRadioPacketReady(void)
{
    si4438_validpkg_flag = 0U;
}
