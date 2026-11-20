/* One and only one peer. Two CRC-protected, committed EEPROM records. */
#include <string.h>
#include "fast_peer.h"
#include "nv_store.h"
#include "link_protocol.h"
#define BASE 0x08080700UL
#define MAGIC 0x35524950UL
static uint32_t peer_id,binding_id,generation;
static uint8_t slot;
static uint8_t Read(uint8_t s,uint32_t r[6])
{
    Nv_ReadBytes(BASE+(uint32_t)s*24UL,(uint8_t *)r,24);
    return (uint8_t)(r[0]==MAGIC&&r[1]&&r[1]!=0xffffffffUL&&r[2]&&r[3]&&
        r[4]==~r[3]&&r[5]==Link_Crc16((const uint8_t *)r,20));
}
void FastPeer_Init(void)
{
    uint32_t a[6],b[6],*r=0;
    uint8_t va=Read(0,a),vb=Read(1,b);
    peer_id=binding_id=generation=0;slot=0;
    if(vb&&(!va||b[3]>a[3])){r=b;slot=1;}else if(va)r=a;
    if(r){peer_id=r[1];binding_id=r[2];generation=r[3];}
}
uint32_t FastPeer_Id(void){return peer_id;}
uint32_t FastPeer_Binding(void){return binding_id;}
uint8_t FastPeer_Accepts(const LinkMessage *m,uint32_t local)
{return (uint8_t)(peer_id&&m->sender_id==peer_id&&m->receiver_id==local&&m->binding==binding_id);}
uint8_t FastPeer_Save(uint32_t peer,uint32_t binding)
{
    uint32_t r[6],zero=0,address=BASE+(uint32_t)(!slot)*24UL;
    if(!peer||peer==0xffffffffUL||!binding||generation>=0xfffffffeUL)return 0;
    if(peer==peer_id&&binding==binding_id)return 1;
    r[0]=MAGIC;r[1]=peer;r[2]=binding;r[3]=generation+1;r[4]=~r[3];
    r[5]=Link_Crc16((const uint8_t *)r,20);
    if(!Nv_WriteBytes(address,(const uint8_t *)&zero,4)||
       !Nv_WriteBytes(address+4,(const uint8_t *)(r+1),20)||
       !Nv_WriteBytes(address,(const uint8_t *)r,4))return 0;
    peer_id=peer;binding_id=binding;++generation;slot=(uint8_t)!slot;return 1;
}
