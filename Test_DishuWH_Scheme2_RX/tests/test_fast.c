#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../User/link_protocol.h"
#define DECL(P) \
void P##Pair_Init(uint8_t,uint32_t);void P##Pair_Begin(uint32_t,uint32_t); \
uint8_t P##Pair_Input(uint32_t,const LinkMessage *); \
uint8_t P##Pair_Output(uint32_t,LinkMessage *);uint8_t P##Pair_Active(void); \
void P##Peer_Init(void);uint32_t P##Peer_Id(void);uint32_t P##Peer_Binding(void); \
uint8_t P##Peer_Save(uint32_t,uint32_t);uint8_t P##Peer_Accepts(const LinkMessage *,uint32_t);
DECL(Tx) DECL(Rx)
static uint8_t txmem[4096],rxmem[4096];
static int fail_after=-1;
void TxRead(uint32_t a,uint8_t *p,uint16_t n)
{assert(a>=0x08080000UL&&a+n<=0x08081000UL);memcpy(p,txmem+a-0x08080000UL,n);}
void RxRead(uint32_t a,uint8_t *p,uint16_t n)
{assert(a>=0x08080000UL&&a+n<=0x08081000UL);memcpy(p,rxmem+a-0x08080000UL,n);}
uint8_t TxWrite(uint32_t a,const uint8_t *p,uint16_t n)
{assert(!(a&3)&&!(n&3));memcpy(txmem+a-0x08080000UL,p,n);return 1;}
uint8_t RxWrite(uint32_t a,const uint8_t *p,uint16_t n)
{
    assert(!(a&3)&&!(n&3));
    if(fail_after==0)return 0;
    if(fail_after>0)--fail_after;
    memcpy(rxmem+a-0x08080000UL,p,n);return 1;
}
int main(void)
{
    LinkMessage req,offer,commit,done,m;
    uint8_t packet[28],copy[28],saved[4096];
    unsigned i,b;
    TxPeer_Init();RxPeer_Init();TxPair_Init(1,111);RxPair_Init(0,222);
    TxPair_Begin(100,7);
    assert(TxPair_Output(100,&req));
    assert(!RxPair_Input(100,&req));assert(!RxPair_Output(115,&offer));
    RxPair_Begin(100,8);
    assert(!RxPair_Input(100,&req));assert(!RxPeer_Id());
    m=req;m.sender_id=333;
    assert(!RxPair_Input(101,&m)); /* no second candidate overwrites first */
    assert(!RxPair_Output(114,&offer));assert(RxPair_Output(115,&offer));
    assert(offer.sender_id==222&&offer.receiver_id==111);
    m=offer;m.receiver_id=999;
    assert(!TxPair_Input(115,&m));
    assert(!TxPair_Input(115,&offer));assert(!TxPeer_Id());
    assert(TxPair_Output(115,&commit));
    m=commit;m.receiver_id=999;assert(!RxPair_Input(115,&m));
    assert(RxPair_Input(115,&commit)==1);
    assert(!RxPair_Input(116,&commit)); /* duplicate: no repeat success flash */
    assert(RxPair_Output(131,&done));
    m=done;m.sender_id=333;assert(!TxPair_Input(131,&m));
    assert(TxPair_Input(131,&done)==2);assert(!TxPair_Active());
    assert(RxPeer_Id()==111&&TxPeer_Id()==222);
    TxPeer_Init();RxPeer_Init();
    assert(RxPeer_Id()==111&&TxPeer_Id()==222);
    puts("PASS actual TX/RX handshake with separate EEPROM; address isolation; candidate lock; reboot");
    m=commit;m.type=LINK_DATA;m.payload=8;m.token=2;
    assert(RxPeer_Accepts(&m,222));
    m.sender_id=333;assert(!RxPeer_Accepts(&m,222));m.sender_id=111;
    m.receiver_id=444;assert(!RxPeer_Accepts(&m,222));m.receiver_id=222;
    ++m.binding;assert(!RxPeer_Accepts(&m,222));--m.binding;
    Link_Encode(packet,&m);assert(Link_Decode(packet,&m));
    for(i=0;i<28;++i)for(b=0;b<8;++b){
        memcpy(copy,packet,28);copy[i]^=(uint8_t)(1U<<b);assert(!Link_Decode(copy,&m));
    }
    puts("PASS one-to-one data gates; 224 single-bit corruptions rejected");
    memcpy(saved,rxmem,sizeof(saved));
    for(i=0;i<3;++i){
        memcpy(rxmem,saved,sizeof(saved));RxPeer_Init();fail_after=(int)i;
        assert(!RxPeer_Save(333,9));fail_after=-1;RxPeer_Init();
        assert(RxPeer_Id()==111&&RxPeer_Binding()==7);
    }
    assert(RxPeer_Save(333,9));RxPeer_Init();assert(RxPeer_Id()==333);
    m=commit;assert(!RxPeer_Accepts(&m,222));
    m.sender_id=333;m.binding=9;assert(RxPeer_Accepts(&m,222));
    puts("PASS interrupted rebind retains old peer; successful rebind rejects old peer");
    return 0;
}
