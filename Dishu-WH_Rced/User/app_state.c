#include "app_state.h"

volatile uint8_t userkey_irq_flag = 0U;
volatile uint8_t si4438_irq_flag = 0U;
volatile uint8_t tim7_irq_flag = 0U;
volatile uint8_t power_off_flag = 0U;
volatile uint8_t low_valtage_flag = 0U;
volatile uint8_t pairing_mode_flag = 0U;

volatile uint8_t si4438_txdata[APP_RADIO_PACKET_SIZE] =
{
    APP_PROTOCOL_MAGIC,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U
};

void AppState_Init(void)
{
    userkey_irq_flag = 0U;
    si4438_irq_flag = 0U;
    tim7_irq_flag = 0U;
    power_off_flag = 0U;
    low_valtage_flag = 0U;
    pairing_mode_flag = 0U;
}

uint8_t AppState_IsRadioPacketReady(void)
{
    return si4438_irq_flag;
}

uint8_t AppState_IsPairingMode(void)
{
    return pairing_mode_flag;
}

void AppState_SetPairingMode(uint8_t enabled)
{
    pairing_mode_flag = (enabled != 0U) ? 1U : 0U;
}
