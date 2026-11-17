#ifndef LINK_PROTOCOL_H
#define LINK_PROTOCOL_H
#include <stdint.h>
#define LINK_PACKET_SIZE 16U
#define LINK_INTERVAL_MS 1000UL
#define LINK_CHANNEL 0U
#define LINK_VERSION 3U
enum { LINK_DATA=0x10, LINK_PAIR_REQUEST=0x20, LINK_PAIR_ACK=0x21 };
enum { LINK_BEACON=0, LINK_SWITCH=1, LINK_USER=2 };
#define LINK_PAIR_ACK_OK 1U
typedef struct {
    uint32_t session;
    uint32_t sender_id;
    uint8_t type;
    uint8_t payload;
    uint8_t sequence;
} LinkMessage;
void Link_Encode(uint8_t packet[LINK_PACKET_SIZE],const LinkMessage *message);
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *message);
uint16_t Link_Crc16(const uint8_t *data,uint8_t length);
uint8_t Link_IsNewer(uint32_t session,uint8_t sequence,
                     uint32_t last_session,uint8_t last_sequence,uint8_t seen);
#endif
