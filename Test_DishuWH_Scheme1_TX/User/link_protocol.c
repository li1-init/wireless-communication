#include "link_protocol.h"
uint16_t Link_Crc16(const uint8_t *p,uint8_t n)
{
    uint16_t c=0xffff;
    uint8_t i,b;
    for(i=0;i<n;++i) { c^=(uint16_t)p[i]<<8; for(b=0;b<8;++b)
        c=(uint16_t)((c&0x8000)?((uint32_t)c<<1)^0x1021:(uint32_t)c<<1); }
    return c;
}
static void Put(uint8_t *p,uint32_t v)
{uint8_t i;for(i=0;i<4;++i)p[i]=(uint8_t)(v>>(8U*i));}
static uint32_t Get(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
void Link_Encode(uint8_t p[28],const LinkMessage *m)
{
    uint16_t c;
    p[0]='T';p[1]='R';p[2]=LINK_VERSION;p[3]=m->type;
    Put(p+4,m->sender_id);Put(p+8,m->receiver_id);Put(p+12,m->session);
    Put(p+16,m->token);Put(p+20,m->binding);p[24]=m->payload;p[25]=0;
    c=Link_Crc16(p,26);p[26]=(uint8_t)(c>>8);p[27]=(uint8_t)c;
}
uint8_t Link_Decode(const uint8_t p[28],LinkMessage *m)
{
    uint16_t c=Link_Crc16(p,26);
    if(p[0]!='T'||p[1]!='R'||p[2]!=5||p[25]||
       p[26]!=(uint8_t)(c>>8)||p[27]!=(uint8_t)c) return 0;
    m->type=p[3];m->sender_id=Get(p+4);m->receiver_id=Get(p+8);
    m->session=Get(p+12);m->token=Get(p+16);m->binding=Get(p+20);m->payload=p[24];
    if(!m->sender_id||m->sender_id==0xffffffffUL||!m->session||!m->token)return 0;
    if(m->type==LINK_PAIR_REQUEST)
        return (uint8_t)(!m->receiver_id&&!m->binding&&!m->payload);
    if(!m->receiver_id||m->receiver_id==0xffffffffUL||!m->binding)return 0;
    if(m->type==LINK_DATA)return (uint8_t)((m->payload>>3)>=1&&(m->payload>>3)<=2);
    return (uint8_t)((m->type==LINK_DATA_ACK||m->type==LINK_PAIR_ACK||
                     m->type==LINK_PAIR_COMMIT||m->type==LINK_PAIR_DONE)&&m->payload==1);
}
