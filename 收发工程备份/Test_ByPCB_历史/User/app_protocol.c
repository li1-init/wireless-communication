#include "app_protocol.h"

static uint8_t pendingCommand = ALARM_CODING;
static uint8_t nextSequence = 1U;

static uint8_t AppProtocol_StatusForCommand(uint8_t command)
{
    if (command == ALARM_TRIG_TB1st)
    {
        return APP_STATUS_ALARM_ACTIVE;
    }
    if (command == ALARM_SWITCHOFF)
    {
        return APP_STATUS_POWER_OFF;
    }
    return 0U;
}

void AppProtocol_SetTxCommand(uint8_t command)
{
    pendingCommand = command;
}

uint8_t AppProtocol_GetTxCommand(void)
{
    return pendingCommand;
}

uint8_t AppProtocol_NextSequence(void)
{
    uint8_t result = nextSequence++;

    if (nextSequence == 0U)
    {
        nextSequence = 1U;
    }
    return result;
}

void AppProtocol_BuildRequest(volatile uint8_t *packet,
                              uint8_t command,
                              uint32_t uid,
                              uint8_t sequence)
{
    packet[0] = APP_PROTOCOL_MAGIC;
    packet[APP_PROTOCOL_CMD_INDEX] = command;
    packet[APP_PROTOCOL_UID_INDEX] = (uint8_t)(uid);
    packet[APP_PROTOCOL_UID_INDEX + 1U] = (uint8_t)(uid >> 8);
    packet[APP_PROTOCOL_UID_INDEX + 2U] = (uint8_t)(uid >> 16);
    packet[APP_PROTOCOL_UID_INDEX + 3U] = (uint8_t)(uid >> 24);
    packet[APP_PROTOCOL_STATUS_INDEX] =
        AppProtocol_StatusForCommand(command);
    packet[APP_PROTOCOL_SEQUENCE_INDEX] = sequence;
}

void AppProtocol_BuildAck(volatile uint8_t *ack,
                          const volatile uint8_t *request)
{
    uint8_t i;

    for (i = 0U; i < APP_RADIO_PACKET_SIZE; ++i)
    {
        ack[i] = request[i];
    }
    ack[APP_PROTOCOL_CMD_INDEX] = ALARM_ACK;
}

uint8_t AppProtocol_IsValid(const volatile uint8_t *packet)
{
    uint32_t uid = AppProtocol_GetUID(packet);

    return (uint8_t)((packet[0] == APP_PROTOCOL_MAGIC) &&
                     (uid != 0U) &&
                     (uid != 0xFFFFFFFFUL));
}

uint8_t AppProtocol_IsAckFor(const volatile uint8_t *packet,
                             uint32_t uid,
                             uint8_t sequence)
{
    return (uint8_t)(AppProtocol_IsValid(packet) &&
                     (AppProtocol_GetCommand(packet) == ALARM_ACK) &&
                     (AppProtocol_GetUID(packet) == uid) &&
                     (AppProtocol_GetSequence(packet) == sequence));
}

uint8_t AppProtocol_GetCommand(const volatile uint8_t *packet)
{
    return packet[APP_PROTOCOL_CMD_INDEX];
}

uint32_t AppProtocol_GetUID(const volatile uint8_t *packet)
{
    return ((uint32_t)packet[APP_PROTOCOL_UID_INDEX]) |
           ((uint32_t)packet[APP_PROTOCOL_UID_INDEX + 1U] << 8) |
           ((uint32_t)packet[APP_PROTOCOL_UID_INDEX + 2U] << 16) |
           ((uint32_t)packet[APP_PROTOCOL_UID_INDEX + 3U] << 24);
}

uint8_t AppProtocol_GetStatus(const volatile uint8_t *packet)
{
    return packet[APP_PROTOCOL_STATUS_INDEX];
}

uint8_t AppProtocol_GetSequence(const volatile uint8_t *packet)
{
    return packet[APP_PROTOCOL_SEQUENCE_INDEX];
}

uint8_t AppProtocol_IsSupportedCommand(uint8_t command)
{
    return (uint8_t)((command == ALARM_CODING) ||
                     (command == ALARM_SWITCHOFF) ||
                     (command == ALARM_TRIG_TB1st));
}
