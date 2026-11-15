#include "app_radio.h"
#include "app_protocol.h"
#include "app_events.h"
#include "board_config.h"
#include "platform.h"
#include "bsp.h"

void AppRadio_Init(void)
{
    SPI_Config(SI4438_SPI);
    vRadio_Init();
}

void AppRadio_StartReceive(void)
{
    vRadio_StartRX_ConstOn(APP_RADIO_PACKET_SIZE, APP_RADIO_CHANNEL_NUMBER);
}

void AppRadio_SendPacket(const volatile uint8_t *packet)
{
    uint8_t txPacket[APP_RADIO_PACKET_SIZE];
    uint8_t i;

    for (i = 0U; i < APP_RADIO_PACKET_SIZE; ++i)
    {
        txPacket[i] = packet[i];
    }

    vRadio_StartTx(APP_RADIO_PACKET_SIZE, txPacket,
                   APP_RADIO_CHANNEL_NUMBER);
}

void AppRadio_ReadPacket(volatile uint8_t *packet)
{
    uint8_t rxPacket[APP_RADIO_PACKET_SIZE];
    uint8_t i;

    si446x_read_rx_fifo(APP_RADIO_PACKET_SIZE, rxPacket);

    for (i = 0U; i < APP_RADIO_PACKET_SIZE; ++i)
    {
        packet[i] = rxPacket[i];
    }
}

void AppRadio_EnterSleep(void)
{
    si446x_change_state(SI446X_STATE_SLEEP);
}

volatile uint8_t *AppRadio_GetReceivedPacket(void)
{
    return fixRadioPacket;
}

void AppRadio_HandleIrq(void)
{
    uint8_t ph_pending;

    si446x_get_ph_status(0);
    ph_pending = Si446xCmd.GET_MODEM_STATUS.MODEM_PEND;

    AppRadio_ReadPacket(fixRadioPacket);

    if ((ph_pending & 0x18U) == 0x10U)
    {
        AppEvent_OnRadioPacketReceived();
    }
    else
    {
        AppRadio_StartReceive();
        EXTI->IMR |= EXTILINE_SI4438_EXITRX;
    }
}
