#ifndef LINK_PROTOCOL_H
#define LINK_PROTOCOL_H
#include <stdint.h>
#define LINK_PACKET_SIZE 16U
#define LINK_INTERVAL_MS 1000UL
#define LINK_CHANNEL 0U
/* Receiver learns one sender ID per power cycle and requires strictly
 * increasing sequence numbers before producing a success indication. */
#define LINK_EXPECTED_SENDER_ID 3155002632UL
enum { LINK_BEACON=0, LINK_SWITCH=1, LINK_USER=2, LINK_CODE=3 };
typedef struct {
    uint32_t sequence;
    uint32_t sender_id;
    uint8_t command;
    uint8_t keys;
} LinkMessage;
void Link_Encode(uint8_t packet[LINK_PACKET_SIZE],const LinkMessage *message);
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *message);
uint16_t Link_Crc16(const uint8_t *data,uint8_t length);
#endif
