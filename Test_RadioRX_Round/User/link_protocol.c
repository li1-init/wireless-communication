#include "link_protocol.h"

uint16_t Link_Crc16(const uint8_t *data,uint8_t length)
{
    uint16_t crc=0xFFFFU;
    uint8_t i,bit;
    for (i=0;i<length;++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (bit=0;bit<8U;++bit)
            crc=(uint16_t)((crc & 0x8000U) ? ((uint32_t)crc << 1)^0x1021U : (uint32_t)crc << 1);
    }
    return crc;
}
static void Put32(uint8_t *p,uint32_t value)
{
    uint8_t i;
    for(i=0;i<4U;++i) p[i]=(uint8_t)(value>>(i*8U));
}
static uint32_t Get32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
void Link_Encode(uint8_t packet[LINK_PACKET_SIZE],const LinkMessage *message)
{
    uint16_t crc;
    packet[0]=0x54U; packet[1]=0x52U; packet[2]=2U; packet[3]=message->command;
    Put32(packet+4,message->session);
    Put32(packet+8,message->sender_id);
    packet[12]=message->keys;
    packet[13]=message->sequence;
    crc=Link_Crc16(packet,14U);
    packet[14]=(uint8_t)(crc>>8);
    packet[15]=(uint8_t)crc;
}
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *message)
{
    uint16_t crc=Link_Crc16(packet,14U);
    if(packet[0]!=0x54U || packet[1]!=0x52U || packet[2]!=2U || packet[3]>LINK_CODE ||
       packet[12]>7U ||
       packet[14]!=(uint8_t)(crc>>8) || packet[15]!=(uint8_t)crc) return 0U;
    message->session=Get32(packet+4);
    message->sender_id=Get32(packet+8);
    message->command=packet[3];
    message->keys=packet[12];
    message->sequence=packet[13];
    return 1U;
}
