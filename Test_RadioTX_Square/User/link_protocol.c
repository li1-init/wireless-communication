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
    packet[0]=0x54U; packet[1]=0x52U; packet[2]=LINK_VERSION; packet[3]=message->type;
    Put32(packet+4,message->session);
    Put32(packet+8,message->sender_id);
    packet[12]=message->payload;
    packet[13]=message->sequence;
    crc=Link_Crc16(packet,14U);
    packet[14]=(uint8_t)(crc>>8);
    packet[15]=(uint8_t)crc;
}
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *message)
{
    uint16_t crc=Link_Crc16(packet,14U);
    if(packet[0]!=0x54U || packet[1]!=0x52U || packet[2]!=LINK_VERSION ||
       !((packet[3]==LINK_DATA && (packet[12]>>3)<=LINK_USER) ||
         (packet[3]==LINK_PAIR_REQUEST && packet[12]==0U) ||
         (packet[3]==LINK_PAIR_ACK && packet[12]==LINK_PAIR_ACK_OK)) ||
       Get32(packet+4)==0U || Get32(packet+8)==0U || Get32(packet+8)==0xFFFFFFFFUL ||
       packet[14]!=(uint8_t)(crc>>8) || packet[15]!=(uint8_t)crc) return 0U;
    message->session=Get32(packet+4);
    message->sender_id=Get32(packet+8);
    message->type=packet[3];
    message->payload=packet[12];
    message->sequence=packet[13];
    return 1U;
}
uint8_t Link_IsNewer(uint32_t session,uint8_t sequence,
                     uint32_t last_session,uint8_t last_sequence,uint8_t seen)
{
    uint8_t difference;
    if(!seen) return 1U;
    if((int32_t)(session-last_session)>0) return 1U;
    if(session!=last_session) return 0U;
    difference=(uint8_t)(sequence-last_sequence);
    return (difference>0U && difference<128U) ? 1U : 0U;
}
