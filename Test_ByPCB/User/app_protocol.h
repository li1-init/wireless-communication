#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

#include <stdint.h>

/*
 * Si4438 频率配置：
 * - 基准频率：433.000 MHz
 * - 信道步进：250 kHz
 * - 当前使用第 4 信道
 * - 实际发射频率：433.000 + 4 * 0.250 = 434.000 MHz
 *
 * 基准频率和信道步进由 radio_config.h 中的无线配置生成，实际信道在这里选择。
 */
#define APP_RADIO_BASE_FREQUENCY_HZ  433000000UL
#define APP_RADIO_CHANNEL_STEP_HZ    250000UL
#define APP_RADIO_CHANNEL_NUMBER     4U
#define APP_RADIO_FREQUENCY_HZ       \
    (APP_RADIO_BASE_FREQUENCY_HZ + \
     (APP_RADIO_CHANNEL_NUMBER * APP_RADIO_CHANNEL_STEP_HZ))

/*
 * Application packet, 8 bytes:
 * [0] 0xA5
 * [1] command
 * [2..5] source UID, little-endian
 * [6] status mask
 * [7] sequence
 */
#define APP_RADIO_PACKET_SIZE        8U
#define APP_PROTOCOL_MAGIC           0xA5U

#define APP_PROTOCOL_CMD_INDEX       1U
#define APP_PROTOCOL_UID_INDEX       2U
#define APP_PROTOCOL_STATUS_INDEX    6U
#define APP_PROTOCOL_SEQUENCE_INDEX  7U

#define APP_STATUS_ALARM_ACTIVE      (1U << 0)
#define APP_STATUS_POWER_OFF         (1U << 1)

typedef enum
{
    ALARM_CODING     = 0xA1, /* Pair request. */
    ALARM_SWITCHOFF  = 0xA2, /* Power key. */
    ALARM_TRIG_TB1st = 0xA3, /* Alarm key. */
    ALARM_ACK        = 0xA6  /* Host acknowledgement. */
} tx_alarm_type;

void AppProtocol_SetTxCommand(uint8_t command);
uint8_t AppProtocol_GetTxCommand(void);
uint8_t AppProtocol_NextSequence(void);

void AppProtocol_BuildRequest(volatile uint8_t *packet,
                              uint8_t command,
                              uint32_t uid,
                              uint8_t sequence);
void AppProtocol_BuildAck(volatile uint8_t *ack,
                          const volatile uint8_t *request);

uint8_t AppProtocol_IsValid(const volatile uint8_t *packet);
uint8_t AppProtocol_IsAckFor(const volatile uint8_t *packet,
                             uint32_t uid,
                             uint8_t sequence);
uint8_t AppProtocol_GetCommand(const volatile uint8_t *packet);
uint32_t AppProtocol_GetUID(const volatile uint8_t *packet);
uint8_t AppProtocol_GetStatus(const volatile uint8_t *packet);
uint8_t AppProtocol_GetSequence(const volatile uint8_t *packet);
uint8_t AppProtocol_IsSupportedCommand(uint8_t command);

#endif /* APP_PROTOCOL_H */
