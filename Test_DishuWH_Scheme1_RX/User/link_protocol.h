#ifndef LINK_PROTOCOL_H
#define LINK_PROTOCOL_H
#include <stdint.h>
#define LINK_PACKET_SIZE 28U
#define LINK_VERSION 5U
#define LINK_CHANNEL 0U
enum {LINK_DATA=0x10,LINK_DATA_ACK=0x11,LINK_PAIR_REQUEST=0x20,
      LINK_PAIR_ACK=0x21,LINK_PAIR_COMMIT=0x22,LINK_PAIR_DONE=0x23};
typedef struct {
    uint32_t sender_id,receiver_id,session,token,binding;
    uint8_t type,payload;
} LinkMessage;
uint16_t Link_Crc16(const uint8_t *data,uint8_t length);
void Link_Encode(uint8_t packet[LINK_PACKET_SIZE],const LinkMessage *m);
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *m);
#endif
