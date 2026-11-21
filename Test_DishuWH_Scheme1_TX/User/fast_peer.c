/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 引入本模块所需的头文件接口。 */
#include <string.h>
/* 引入本模块所需的头文件接口。 */
#include "fast_peer.h"
/* 引入本模块所需的头文件接口。 */
#include "nv_store.h"
/* 引入本模块所需的头文件接口。 */
#include "link_protocol.h"
/* 定义编译期常量或接口宏。 */
#define BASE 0x08080700UL
/* 定义编译期常量或接口宏。 */
#define MAGIC 0x35524950UL
/* 执行当前语句。 */
static uint32_t peer_id,binding_id,generation;
/* 定义或初始化本模块使用的状态数据。 */
static uint8_t slot;
/* 进入本模块的函数实现。 */
static uint8_t Read(uint8_t s,uint32_t r[6])
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    Nv_ReadBytes(BASE+(uint32_t)s*24UL,(uint8_t *)r,24);
/* 返回当前函数的处理结果。 */
    return (uint8_t)(r[0]==MAGIC&&r[1]&&r[1]!=0xffffffffUL&&r[2]&&r[3]&&
/* 执行当前语句。 */
        r[4]==~r[3]&&r[5]==Link_Crc16((const uint8_t *)r,20));
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
void FastPeer_Init(void)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t a[6],b[6],*r=0;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t va=Read(0,a),vb=Read(1,b);
/* 更新运行状态或计算结果。 */
    peer_id=binding_id=generation=0;slot=0;
/* 进入本模块的函数实现。 */
    if(vb&&(!va||b[3]>a[3])){r=b;slot=1;}else if(va)r=a;
/* 进入本模块的函数实现。 */
    if(r){peer_id=r[1];binding_id=r[2];generation=r[3];}
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
uint32_t FastPeer_Id(void){return peer_id;}
/* 进入本模块的函数实现。 */
uint32_t FastPeer_Binding(void){return binding_id;}
/* 进入本模块的函数实现。 */
uint8_t FastPeer_Accepts(const LinkMessage *m,uint32_t local)
/* 说明本行代码在当前模块中的处理作用。 */
{return (uint8_t)(peer_id&&m->sender_id==peer_id&&m->receiver_id==local&&m->binding==binding_id);}
/* 进入本模块的函数实现。 */
uint8_t FastPeer_Save(uint32_t peer,uint32_t binding)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t r[6],zero=0,address=BASE+(uint32_t)(!slot)*24UL;
/* 根据运行条件选择处理分支。 */
    if(!peer||peer==0xffffffffUL||!binding||generation>=0xfffffffeUL)return 0;
/* 根据运行条件选择处理分支。 */
    if(peer==peer_id&&binding==binding_id)return 1;
/* 执行当前语句。 */
    r[0]=MAGIC;r[1]=peer;r[2]=binding;r[3]=generation+1;r[4]=~r[3];
/* 执行当前语句。 */
    r[5]=Link_Crc16((const uint8_t *)r,20);
/* 根据运行条件选择处理分支。 */
    if(!Nv_WriteBytes(address,(const uint8_t *)&zero,4)||
/* 说明本行代码在当前模块中的处理作用。 */
       !Nv_WriteBytes(address+4,(const uint8_t *)(r+1),20)||
/* 执行当前语句。 */
       !Nv_WriteBytes(address,(const uint8_t *)r,4))return 0;
/* 更新运行状态或计算结果。 */
    peer_id=peer;binding_id=binding;++generation;slot=(uint8_t)!slot;return 1;
/* 结束当前代码块。 */
}
