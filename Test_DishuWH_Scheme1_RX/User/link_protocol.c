/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 引入本模块所需的头文件接口。 */
#include "link_protocol.h"
/* 进入本模块的函数实现。 */
uint16_t Link_Crc16(const uint8_t *p,uint8_t n)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t c=0xffff;
/* 执行当前语句。 */
    uint8_t i,b;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<n;++i) { c^=(uint16_t)p[i]<<8; for(b=0;b<8;++b)
/* 更新运行状态或计算结果。 */
        c=(uint16_t)((c&0x8000)?((uint32_t)c<<1)^0x1021:(uint32_t)c<<1); }
/* 返回当前函数的处理结果。 */
    return c;
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
static void Put(uint8_t *p,uint32_t v)
/* 说明本行代码在当前模块中的处理作用。 */
{uint8_t i;for(i=0;i<4;++i)p[i]=(uint8_t)(v>>(8U*i));}
/* 进入本模块的函数实现。 */
static uint32_t Get(const uint8_t *p)
/* 说明本行代码在当前模块中的处理作用。 */
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
/* 进入本模块的函数实现。 */
void Link_Encode(uint8_t p[28],const LinkMessage *m)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t c;
/* 执行当前语句。 */
    p[0]='T';p[1]='R';p[2]=LINK_VERSION;p[3]=m->type;
/* 调用相关模块接口完成当前处理步骤。 */
    Put(p+4,m->sender_id);Put(p+8,m->receiver_id);Put(p+12,m->session);
/* 调用相关模块接口完成当前处理步骤。 */
    Put(p+16,m->token);Put(p+20,m->binding);p[24]=m->payload;p[25]=0;
/* 更新运行状态或计算结果。 */
    c=Link_Crc16(p,26);p[26]=(uint8_t)(c>>8);p[27]=(uint8_t)c;
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
uint8_t Link_Decode(const uint8_t p[28],LinkMessage *m)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t c=Link_Crc16(p,26);
/* 根据运行条件选择处理分支。 */
    if(p[0]!='T'||p[1]!='R'||p[2]!=5||p[25]||
/* 执行当前语句。 */
       p[26]!=(uint8_t)(c>>8)||p[27]!=(uint8_t)c) return 0;
/* 更新运行状态或计算结果。 */
    m->type=p[3];m->sender_id=Get(p+4);m->receiver_id=Get(p+8);
/* 更新运行状态或计算结果。 */
    m->session=Get(p+12);m->token=Get(p+16);m->binding=Get(p+20);m->payload=p[24];
/* 根据运行条件选择处理分支。 */
    if(!m->sender_id||m->sender_id==0xffffffffUL||!m->session||!m->token)return 0;
/* 进入本模块的函数实现。 */
    if(m->type==LINK_PAIR_REQUEST)
/* 返回当前函数的处理结果。 */
        return (uint8_t)(!m->receiver_id&&!m->binding&&!m->payload);
/* 根据运行条件选择处理分支。 */
    if(!m->receiver_id||m->receiver_id==0xffffffffUL||!m->binding)return 0;
/* 根据运行条件选择处理分支。 */
    if(m->type==LINK_DATA)return (uint8_t)((m->payload>>3)>=1&&(m->payload>>3)<=7);
/* 返回当前函数的处理结果。 */
    return (uint8_t)((m->type==LINK_DATA_ACK||m->type==LINK_PAIR_ACK||
/* 更新运行状态或计算结果。 */
                     m->type==LINK_PAIR_COMMIT||m->type==LINK_PAIR_DONE)&&m->payload==1);
/* 结束当前代码块。 */
}
