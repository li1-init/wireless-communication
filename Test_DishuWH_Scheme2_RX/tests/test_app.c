#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../User/app_fast.h"
#include "../User/fast_pair.h"
#include "../User/fast_peer.h"
#include "../User/key_input.h"
#include "../User/radio_link.h"
volatile RadioLinkStatus g_radio;
static uint8_t memory[4096],incoming[28],has_packet,raw,leds,active;
static uint32_t now,epoch,begins,sends;
static LinkMessage outgoing;
void Nv_ReadBytes(uint32_t a,uint8_t *p,uint16_t n){memcpy(p,memory+a-0x08080000UL,n);}
uint8_t Nv_WriteBytes(uint32_t a,const uint8_t *p,uint16_t n)
{memcpy(memory+a-0x08080000UL,p,n);return 1;}
uint32_t Board_Millis(void){return now;}
void Board_DelayMs(uint32_t t){now+=t;}
uint8_t Board_Init(void){return 1;}
uint32_t Board_DeviceId(void){return BOARD_IS_TX?111:222;}
uint8_t Board_Keys(void){return raw;}
uint8_t SessionStore_Next(uint32_t *s){*s=++epoch;return 1;}
uint8_t Radio_Init(void){return 1;}
uint8_t Radio_StartReceive(void){return 1;}
void Radio_Shutdown(void){}
int Radio_PollReceive(uint8_t *p)
{if(!has_packet)return 0;memcpy(p,incoming,28);has_packet=0;return 1;}
uint8_t Radio_Send(const uint8_t *p){assert(Link_Decode(p,&outgoing));++sends;return 1;}
void Indicators_Startup(void){}
void Indicators_Ready(void){}
void Indicators_Stop(void){leds=0;}
void Indicators_PairStart(uint32_t t){(void)t;leds=4;}
void Indicators_PairSuccess(uint32_t t){(void)t;leds=4;}
void Indicators_RxPulse(uint32_t t){(void)t;leds=2;}
void Indicators_KeyAck(uint32_t t,uint8_t key){(void)t;leds=(uint8_t)(0x80U|key);}
void Indicators_Fault(uint32_t t,uint8_t e){(void)t;(void)e;}
void FastPair_Init(uint8_t t,uint32_t id){(void)t;(void)id;active=0;}
void FastPair_Begin(uint32_t t,uint32_t s){(void)t;(void)s;assert(leds==4);++begins;active=1;}
void FastPair_Stop(void){active=0;}
uint8_t FastPair_Active(void){return active;}
uint8_t FastPair_Expired(uint32_t t){(void)t;return 0;}
uint8_t FastPair_Output(uint32_t t,LinkMessage *m){(void)t;(void)m;return 0;}
uint8_t FastPair_Input(uint32_t t,const LinkMessage *m){(void)t;(void)m;return 0;}
static void Poll(uint32_t t,uint8_t keys){now=t;raw=keys;AppFast_Poll();}
static void Inject(LinkMessage *m){Link_Encode(incoming,m);has_packet=1;AppFast_Poll();}
int main(void)
{
    LinkMessage m={0},bad;
    AppFast_Init();assert(FastPeer_Save(BOARD_IS_TX?222:111,9));
#if BOARD_IS_TX
    Poll(100,1);Poll(125,1);assert(sends==1&&outgoing.type==LINK_DATA);
    m=outgoing;m.type=LINK_DATA_ACK;m.sender_id=222;m.receiver_id=111;m.payload=1;
    bad=m;bad.sender_id=333;Inject(&bad);assert(!g_link.ack_count&&leds==0);
    bad=m;bad.receiver_id=444;Inject(&bad);assert(!g_link.ack_count);
    Inject(&m);assert(g_link.ack_count==1&&leds==0x81);
    Poll(150,0);Poll(175,0);
    Poll(200,KEY_W1_MASK);Poll(225,KEY_W1_MASK);assert(sends==2&&leds==0);
    m=outgoing;m.type=LINK_DATA_ACK;m.sender_id=222;m.receiver_id=111;m.payload=1;
    bad=m;bad.token++;Inject(&bad);assert(g_link.ack_count==1&&leds==0);
    Inject(&m);assert(g_link.ack_count==2&&leds==0x88);
    Poll(250,0);Poll(275,0);
#else
    m.type=LINK_DATA;m.sender_id=111;m.receiver_id=222;m.binding=9;
    m.session=7;m.token=1;m.payload=8;
    bad=m;bad.sender_id=333;Inject(&bad);assert(!g_link.rx_valid_count&&!sends);
    bad=m;bad.receiver_id=444;Inject(&bad);assert(!g_link.rx_valid_count&&!sends);
    Inject(&m);assert(g_link.rx_valid_count==1&&sends==1&&leds==2);
    assert(outgoing.type==LINK_DATA_ACK&&outgoing.sender_id==222&&outgoing.receiver_id==111);
    Inject(&m);assert(g_link.rx_valid_count==1&&sends==2); /* retry ACK, no repeat indication */
#endif
    Poll(500,0);Poll(525,0);
    Poll(600,4);Poll(625,4);Poll(800,0);Poll(825,0);assert(!begins);
    Poll(1000,4);Poll(1025,4);Poll(3024,4);assert(!begins);
    Poll(3025,4);assert(begins==1&&leds==4);
    Poll(6000,4);assert(begins==1); /* long hold only once */
#if !BOARD_IS_TX
    Poll(6100,0);Poll(6125,0);Poll(6200,4);Poll(6225,4);Poll(8225,4);
    assert(!active&&leds==0); /* receiver exit also requires a fresh long press */
#endif
    puts(BOARD_IS_TX?"PASS production fast TX: isolated ACK and 2s CODE threshold":
                       "PASS production fast RX: isolated data, duplicate ACK, 2s CODE entry/exit");
    return 0;
}
