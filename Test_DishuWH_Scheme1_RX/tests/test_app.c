/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 引入本模块所需的头文件接口。 */
#include <assert.h>
/* 引入本模块所需的头文件接口。 */
#include <stdio.h>
/* 引入本模块所需的头文件接口。 */
#include <string.h>
/* 引入本模块所需的头文件接口。 */
#include "../User/app_fast.h"
/* 引入本模块所需的头文件接口。 */
#include "../User/fast_pair.h"
/* 引入本模块所需的头文件接口。 */
#include "../User/fast_peer.h"
/* 引入本模块所需的头文件接口。 */
#include "../User/key_input.h"
/* 引入本模块所需的头文件接口。 */
#include "../User/radio_link.h"
/* 定义或初始化本模块使用的状态数据。 */
volatile RadioLinkStatus g_radio;
/* 定义或初始化本模块使用的状态数据。 */
static uint8_t memory[4096],incoming[28],has_packet,raw,leds,active;
/* 执行当前语句。 */
static uint32_t now,epoch,begins,sends;
/* 定义或初始化本模块使用的状态数据。 */
static LinkMessage outgoing;
/* 进入本模块的函数实现。 */
void Nv_ReadBytes(uint32_t a,uint8_t *p,uint16_t n){memcpy(p,memory+a-0x08080000UL,n);}
/* 进入本模块的函数实现。 */
uint8_t Nv_WriteBytes(uint32_t a,const uint8_t *p,uint16_t n)
/* 说明本行代码在当前模块中的处理作用。 */
{memcpy(memory+a-0x08080000UL,p,n);return 1;}
/* 进入本模块的函数实现。 */
uint32_t Board_Millis(void){return now;}
/* 进入本模块的函数实现。 */
void Board_DelayMs(uint32_t t){now+=t;}
/* 进入本模块的函数实现。 */
uint8_t Board_Init(void){return 1;}
/* 进入本模块的函数实现。 */
uint32_t Board_DeviceId(void){return BOARD_IS_TX?111:222;}
/* 进入本模块的函数实现。 */
uint8_t Board_Keys(void){return raw;}
/* 进入本模块的函数实现。 */
uint8_t SessionStore_Next(uint32_t *s){*s=++epoch;return 1;}
/* 进入本模块的函数实现。 */
uint8_t Radio_Init(void){return 1;}
/* 进入本模块的函数实现。 */
uint8_t Radio_StartReceive(void){return 1;}
/* 进入本模块的函数实现。 */
void Radio_Shutdown(void){}
/* 进入本模块的函数实现。 */
int Radio_PollReceive(uint8_t *p)
/* 说明本行代码在当前模块中的处理作用。 */
{if(!has_packet)return 0;memcpy(p,incoming,28);has_packet=0;return 1;}
/* 进入本模块的函数实现。 */
uint8_t Radio_Send(const uint8_t *p){assert(Link_Decode(p,&outgoing));++sends;return 1;}
/* 进入本模块的函数实现。 */
void Indicators_Startup(void){}
/* 进入本模块的函数实现。 */
void Indicators_Ready(void){}
/* 进入本模块的函数实现。 */
void Indicators_Stop(void){leds=0;}
/* 进入本模块的函数实现。 */
void Indicators_PairStart(uint32_t t){(void)t;leds=4;}
/* 进入本模块的函数实现。 */
void Indicators_PairSuccess(uint32_t t){(void)t;leds=4;}
/* 进入本模块的函数实现。 */
void Indicators_RxPulse(uint32_t t){(void)t;leds=2;}
/* 进入本模块的函数实现。 */
void Indicators_KeyAck(uint32_t t,uint8_t key){(void)t;leds=(uint8_t)(0x80U|key);}
/* 进入本模块的函数实现。 */
void Indicators_CommandAck(uint32_t t,uint8_t command){(void)t;leds=(uint8_t)(0x40U|command);}
/* 进入本模块的函数实现。 */
void Indicators_Fault(uint32_t t,uint8_t e){(void)t;(void)e;}
/* 进入本模块的函数实现。 */
void FastPair_Init(uint8_t t,uint32_t id){(void)t;(void)id;active=0;}
/* 进入本模块的函数实现。 */
void FastPair_Begin(uint32_t t,uint32_t s){(void)t;(void)s;assert(leds==4);++begins;active=1;}
/* 进入本模块的函数实现。 */
void FastPair_Stop(void){active=0;}
/* 进入本模块的函数实现。 */
uint8_t FastPair_Active(void){return active;}
/* 进入本模块的函数实现。 */
uint8_t FastPair_Expired(uint32_t t){(void)t;return 0;}
/* 进入本模块的函数实现。 */
uint8_t FastPair_Output(uint32_t t,LinkMessage *m){(void)t;(void)m;return 0;}
/* 进入本模块的函数实现。 */
uint8_t FastPair_Input(uint32_t t,const LinkMessage *m){(void)t;(void)m;return 0;}
/* 进入本模块的函数实现。 */
static void Poll(uint32_t t,uint8_t keys){now=t;raw=keys;AppFast_Poll();}
/* 进入本模块的函数实现。 */
static void Inject(LinkMessage *m){Link_Encode(incoming,m);has_packet=1;AppFast_Poll();}
/* 进入本模块的函数实现。 */
int main(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    LinkMessage m={0},bad;
/* 调用相关模块接口完成当前处理步骤。 */
    AppFast_Init();assert(FastPeer_Save(BOARD_IS_TX?222:111,9));
/* 根据编译条件选择对应实现。 */
#if BOARD_IS_TX
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(100,1);Poll(125,1);assert(sends==1&&outgoing.type==LINK_DATA);
/* 更新运行状态或计算结果。 */
    m=outgoing;m.type=LINK_DATA_ACK;m.sender_id=222;m.receiver_id=111;m.payload=1;
/* 更新运行状态或计算结果。 */
    bad=m;bad.sender_id=333;Inject(&bad);assert(!g_link.ack_count&&leds==0);
/* 更新运行状态或计算结果。 */
    bad=m;bad.receiver_id=444;Inject(&bad);assert(!g_link.ack_count);
/* 调用相关模块接口完成当前处理步骤。 */
    Inject(&m);assert(g_link.ack_count==1&&leds==0x81);
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(150,0);Poll(175,0);
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(200,KEY_W1_MASK);Poll(225,KEY_W1_MASK);assert(sends==2&&leds==0);
/* 更新运行状态或计算结果。 */
    m=outgoing;m.type=LINK_DATA_ACK;m.sender_id=222;m.receiver_id=111;m.payload=1;
/* 更新运行状态或计算结果。 */
    bad=m;bad.token++;Inject(&bad);assert(g_link.ack_count==1&&leds==0);
/* 调用相关模块接口完成当前处理步骤。 */
    Inject(&m);assert(g_link.ack_count==2&&leds==0x88);
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(250,0);Poll(275,0);
/* 根据编译条件选择对应实现。 */
#else
/* 执行当前语句。 */
    m.type=LINK_DATA;m.sender_id=111;m.receiver_id=222;m.binding=9;
/* 执行当前语句。 */
    m.session=7;m.token=1;m.payload=8;
/* 更新运行状态或计算结果。 */
    bad=m;bad.sender_id=333;Inject(&bad);assert(!g_link.rx_valid_count&&!sends);
/* 更新运行状态或计算结果。 */
    bad=m;bad.receiver_id=444;Inject(&bad);assert(!g_link.rx_valid_count&&!sends);
/* 调用相关模块接口完成当前处理步骤。 */
    Inject(&m);assert(g_link.rx_valid_count==1&&sends==1&&leds==2);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(outgoing.type==LINK_DATA_ACK&&outgoing.sender_id==222&&outgoing.receiver_id==111);
/* 调用相关模块接口完成当前处理步骤。 */
    Inject(&m);assert(g_link.rx_valid_count==1&&sends==2); /* retry ACK, no repeat indication */
/* 执行当前语句。 */
    m.payload=(uint8_t)(4U<<3);m.token=2;
/* 调用相关模块接口完成当前处理步骤。 */
    Inject(&m);assert(g_link.rx_valid_count==2&&sends==3&&leds==0x44);
/* 根据编译条件选择对应实现。 */
#endif
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(500,0);Poll(525,0);
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(600,4);Poll(625,4);Poll(800,0);Poll(825,0);assert(!begins);
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(1000,4);Poll(1025,4);Poll(3024,4);assert(!begins);
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(3025,4);assert(begins==1&&leds==4);
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(6000,4);assert(begins==1); /* long hold only once */
/* 根据编译条件选择对应实现。 */
#if !BOARD_IS_TX
/* 调用相关模块接口完成当前处理步骤。 */
    Poll(6100,0);Poll(6125,0);Poll(6200,4);Poll(6225,4);Poll(8225,4);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!active&&leds==0); /* receiver exit also requires a fresh long press */
/* 根据编译条件选择对应实现。 */
#endif
/* 调用相关模块接口完成当前处理步骤。 */
    puts(BOARD_IS_TX?"PASS production fast TX: isolated ACK and 2s CODE threshold":
/* 执行当前语句。 */
                       "PASS production fast RX: isolated data, duplicate ACK, 2s CODE entry/exit");
/* 返回当前函数的处理结果。 */
    return 0;
/* 结束当前代码块。 */
}
