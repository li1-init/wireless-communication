/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 引入本模块所需的头文件接口。 */
#include <assert.h>
/* 引入本模块所需的头文件接口。 */
#include <stdio.h>
/* 引入本模块所需的头文件接口。 */
#include <string.h>
/* 引入本模块所需的头文件接口。 */
#include "../User/link_protocol.h"
/* 定义编译期常量或接口宏。 */
#define DECL(P) /* 定义带角色前缀的测试接口声明宏。 */ \
void P##Pair_Init(uint8_t,uint32_t);void P##Pair_Begin(uint32_t,uint32_t); /* 声明对码初始化和启动接口。 */ \
uint8_t P##Pair_Input(uint32_t,const LinkMessage *); /* 声明对码输入接口。 */ \
uint8_t P##Pair_Output(uint32_t,LinkMessage *);uint8_t P##Pair_Active(void); /* 声明对码输出和状态接口。 */ \
void P##Peer_Init(void);uint32_t P##Peer_Id(void);uint32_t P##Peer_Binding(void); /* 声明对端初始化和绑定信息接口。 */ \
uint8_t P##Peer_Save(uint32_t,uint32_t);uint8_t P##Peer_Accepts(const LinkMessage *,uint32_t);
/* 进入本模块的函数实现。 */
DECL(Tx) DECL(Rx)
/* 定义或初始化本模块使用的状态数据。 */
static uint8_t txmem[4096],rxmem[4096];
/* 定义或初始化本模块使用的状态数据。 */
static int fail_after=-1;
/* 进入本模块的函数实现。 */
void TxRead(uint32_t a,uint8_t *p,uint16_t n)
/* 说明本行代码在当前模块中的处理作用。 */
{assert(a>=0x08080000UL&&a+n<=0x08081000UL);memcpy(p,txmem+a-0x08080000UL,n);}
/* 进入本模块的函数实现。 */
void RxRead(uint32_t a,uint8_t *p,uint16_t n)
/* 说明本行代码在当前模块中的处理作用。 */
{assert(a>=0x08080000UL&&a+n<=0x08081000UL);memcpy(p,rxmem+a-0x08080000UL,n);}
/* 进入本模块的函数实现。 */
uint8_t TxWrite(uint32_t a,const uint8_t *p,uint16_t n)
/* 说明本行代码在当前模块中的处理作用。 */
{assert(!(a&3)&&!(n&3));memcpy(txmem+a-0x08080000UL,p,n);return 1;}
/* 进入本模块的函数实现。 */
uint8_t RxWrite(uint32_t a,const uint8_t *p,uint16_t n)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!(a&3)&&!(n&3));
/* 根据运行条件选择处理分支。 */
    if(fail_after==0)return 0;
/* 根据运行条件选择处理分支。 */
    if(fail_after>0)--fail_after;
/* 调用相关模块接口完成当前处理步骤。 */
    memcpy(rxmem+a-0x08080000UL,p,n);return 1;
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
int main(void)
/* 开始当前代码块。 */
{
/* 执行当前语句。 */
    LinkMessage req,offer,commit,done,m;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t packet[28],copy[28],saved[4096];
/* 执行当前语句。 */
    unsigned i,b;
/* 调用相关模块接口完成当前处理步骤。 */
    TxPeer_Init();RxPeer_Init();TxPair_Init(1,111);RxPair_Init(0,222);
/* 调用相关模块接口完成当前处理步骤。 */
    TxPair_Begin(100,7);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(TxPair_Output(100,&req));
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!RxPair_Input(100,&req));assert(!RxPair_Output(115,&offer));
/* 调用相关模块接口完成当前处理步骤。 */
    RxPair_Begin(100,8);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!RxPair_Input(100,&req));assert(!RxPeer_Id());
/* 更新运行状态或计算结果。 */
    m=req;m.sender_id=333;
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!RxPair_Input(101,&m)); /* no second candidate overwrites first */
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!RxPair_Output(114,&offer));assert(RxPair_Output(115,&offer));
/* 调用相关模块接口完成当前处理步骤。 */
    assert(offer.sender_id==222&&offer.receiver_id==111);
/* 更新运行状态或计算结果。 */
    m=offer;m.receiver_id=999;
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!TxPair_Input(115,&m));
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!TxPair_Input(115,&offer));assert(!TxPeer_Id());
/* 调用相关模块接口完成当前处理步骤。 */
    assert(TxPair_Output(115,&commit));
/* 更新运行状态或计算结果。 */
    m=commit;m.receiver_id=999;assert(!RxPair_Input(115,&m));
/* 调用相关模块接口完成当前处理步骤。 */
    assert(RxPair_Input(115,&commit)==1);
/* 调用相关模块接口完成当前处理步骤。 */
    assert(!RxPair_Input(116,&commit)); /* duplicate: no repeat success flash */
/* 调用相关模块接口完成当前处理步骤。 */
    assert(RxPair_Output(131,&done));
/* 更新运行状态或计算结果。 */
    m=done;m.sender_id=333;assert(!TxPair_Input(131,&m));
/* 调用相关模块接口完成当前处理步骤。 */
    assert(TxPair_Input(131,&done)==2);assert(!TxPair_Active());
/* 调用相关模块接口完成当前处理步骤。 */
    assert(RxPeer_Id()==111&&TxPeer_Id()==222);
/* 调用相关模块接口完成当前处理步骤。 */
    TxPeer_Init();RxPeer_Init();
/* 调用相关模块接口完成当前处理步骤。 */
    assert(RxPeer_Id()==111&&TxPeer_Id()==222);
/* 调用相关模块接口完成当前处理步骤。 */
    puts("PASS actual TX/RX handshake with separate EEPROM; address isolation; candidate lock; reboot");
/* 更新运行状态或计算结果。 */
    m=commit;m.type=LINK_DATA;m.payload=8;m.token=2;
/* 调用相关模块接口完成当前处理步骤。 */
    assert(RxPeer_Accepts(&m,222));
/* 执行当前语句。 */
    m.sender_id=333;assert(!RxPeer_Accepts(&m,222));m.sender_id=111;
/* 执行当前语句。 */
    m.receiver_id=444;assert(!RxPeer_Accepts(&m,222));m.receiver_id=222;
/* 更新运行状态或计算结果。 */
    ++m.binding;assert(!RxPeer_Accepts(&m,222));--m.binding;
/* 调用相关模块接口完成当前处理步骤。 */
    Link_Encode(packet,&m);assert(Link_Decode(packet,&m));
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<28;++i)for(b=0;b<8;++b){
/* 调用相关模块接口完成当前处理步骤。 */
        memcpy(copy,packet,28);copy[i]^=(uint8_t)(1U<<b);assert(!Link_Decode(copy,&m));
/* 结束当前代码块。 */
    }
/* 调用相关模块接口完成当前处理步骤。 */
    puts("PASS one-to-one data gates; 224 single-bit corruptions rejected");
/* 调用相关模块接口完成当前处理步骤。 */
    memcpy(saved,rxmem,sizeof(saved));
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<3;++i){
/* 调用相关模块接口完成当前处理步骤。 */
        memcpy(rxmem,saved,sizeof(saved));RxPeer_Init();fail_after=(int)i;
/* 调用相关模块接口完成当前处理步骤。 */
        assert(!RxPeer_Save(333,9));fail_after=-1;RxPeer_Init();
/* 调用相关模块接口完成当前处理步骤。 */
        assert(RxPeer_Id()==111&&RxPeer_Binding()==7);
/* 结束当前代码块。 */
    }
/* 调用相关模块接口完成当前处理步骤。 */
    assert(RxPeer_Save(333,9));RxPeer_Init();assert(RxPeer_Id()==333);
/* 更新运行状态或计算结果。 */
    m=commit;assert(!RxPeer_Accepts(&m,222));
/* 执行当前语句。 */
    m.sender_id=333;m.binding=9;assert(RxPeer_Accepts(&m,222));
/* 调用相关模块接口完成当前处理步骤。 */
    puts("PASS interrupted rebind retains old peer; successful rebind rejects old peer");
/* 返回当前函数的处理结果。 */
    return 0;
/* 结束当前代码块。 */
}
