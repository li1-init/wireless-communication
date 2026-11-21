/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 引入本模块所需的头文件接口。 */
#include <string.h>
/* 引入本模块所需的头文件接口。 */
#include "fast_pair.h"
/* 引入本模块所需的头文件接口。 */
#include "fast_peer.h"
/* 执行当前语句。 */
static uint8_t tx,active,stage,pending,committed,candidate_valid;
/* 执行当前语句。 */
static uint32_t local,started,last_send,reply_at,candidate_at;
/* 执行当前语句。 */
static LinkMessage transaction,queued;
/* 进入本模块的函数实现。 */
void FastPair_Init(uint8_t transmitter,uint32_t id)
/* 说明本行代码在当前模块中的处理作用。 */
{tx=transmitter;local=id;active=stage=pending=committed=candidate_valid=0;}
/* 进入本模块的函数实现。 */
void FastPair_Stop(void){active=pending=candidate_valid=committed=0;}
/* 进入本模块的函数实现。 */
void FastPair_Begin(uint32_t now,uint32_t session)
/* 开始当前代码块。 */
{
/* 调用相关模块接口完成当前处理步骤。 */
    FastPair_Stop();active=1;started=now;last_send=now-150UL;stage=0;
/* 调用相关模块接口完成当前处理步骤。 */
    memset(&transaction,0,sizeof(transaction));
/* 执行当前语句。 */
    transaction.sender_id=local;transaction.session=session;transaction.token=1;
/* 执行当前语句。 */
    transaction.type=LINK_PAIR_REQUEST;
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
uint8_t FastPair_Active(void){return active;}
/* 进入本模块的函数实现。 */
uint8_t FastPair_Expired(uint32_t now)
/* 开始当前代码块。 */
{
/* 进入本模块的函数实现。 */
    if(tx&&active&&(uint32_t)(now-started)>=10000UL){FastPair_Stop();return 1;}
/* 返回当前函数的处理结果。 */
    return 0;
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
static void Reply(uint32_t now,const LinkMessage *m,uint8_t type)
/* 开始当前代码块。 */
{
/* 更新运行状态或计算结果。 */
    queued=*m;queued.type=type;queued.sender_id=local;queued.receiver_id=m->sender_id;
/* 执行当前语句。 */
    queued.payload=1;pending=1;reply_at=now;
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
uint8_t FastPair_Output(uint32_t now,LinkMessage *out)
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(!active)return 0;
/* 进入本模块的函数实现。 */
    if(pending&&(uint32_t)(now-reply_at)>=15UL){*out=queued;pending=0;return 1;}
/* 进入本模块的函数实现。 */
    if(tx&&(uint32_t)(now-last_send)>=150UL) {
/* 更新运行状态或计算结果。 */
        last_send=now;*out=transaction;
/* 进入本模块的函数实现。 */
        if(stage){out->type=LINK_PAIR_COMMIT;out->payload=1;}
/* 返回当前函数的处理结果。 */
        return 1;
/* 结束当前代码块。 */
    }
/* 返回当前函数的处理结果。 */
    return 0;
/* 结束当前代码块。 */
}
/* 进入本模块的函数实现。 */
uint8_t FastPair_Input(uint32_t now,const LinkMessage *m)
/* 开始当前代码块。 */
{
/* 根据运行条件选择处理分支。 */
    if(!active||m->sender_id==local)return 0;
/* 进入本模块的函数实现。 */
    if(tx) {
/* 根据运行条件选择处理分支。 */
        if(m->receiver_id!=local||m->session!=transaction.session||
/* 执行当前语句。 */
           m->token!=transaction.token)return 0;
/* 进入本模块的函数实现。 */
        if(!stage&&m->type==LINK_PAIR_ACK&&m->binding==transaction.session) {
/* 执行当前语句。 */
            transaction.receiver_id=m->sender_id;transaction.binding=m->binding;
/* 更新运行状态或计算结果。 */
            stage=1;last_send=now-150UL;
/* 说明本行代码在当前模块中的处理作用。 */
        } else if(stage&&m->type==LINK_PAIR_DONE&&
/* 更新运行状态或计算结果。 */
                  m->sender_id==transaction.receiver_id&&m->binding==transaction.binding) {
/* 根据运行条件选择处理分支。 */
            if(!FastPeer_Save(m->sender_id,m->binding))return 0;
/* 调用相关模块接口完成当前处理步骤。 */
            FastPair_Stop();return 2;
/* 结束当前代码块。 */
        }
/* 返回当前函数的处理结果。 */
        return 0;
/* 结束当前代码块。 */
    }
/* 进入本模块的函数实现。 */
    if(m->type==LINK_PAIR_REQUEST) {
        /* First candidate wins until timeout or commit; no cross-device overwrite mid-handshake. */
/* 根据运行条件选择处理分支。 */
        if(candidate_valid&&(uint32_t)(now-candidate_at)<2000UL&&
/* 说明本行代码在当前模块中的处理作用。 */
           (m->sender_id!=transaction.sender_id||m->session!=transaction.session||
/* 执行当前语句。 */
            m->token!=transaction.token))return 0;
/* 根据运行条件选择处理分支。 */
        if(!candidate_valid||m->sender_id!=transaction.sender_id||
/* 执行当前语句。 */
           m->session!=transaction.session||m->token!=transaction.token)committed=0;
/* 更新运行状态或计算结果。 */
        transaction=*m;transaction.binding=m->session;
/* 更新运行状态或计算结果。 */
        candidate_valid=1;candidate_at=now;
/* 调用相关模块接口完成当前处理步骤。 */
        Reply(now,&transaction,LINK_PAIR_ACK);return 0;
/* 结束当前代码块。 */
    }
/* 根据运行条件选择处理分支。 */
    if(m->type!=LINK_PAIR_COMMIT||!candidate_valid||m->receiver_id!=local||
/* 说明本行代码在当前模块中的处理作用。 */
       m->sender_id!=transaction.sender_id||m->session!=transaction.session||
/* 执行当前语句。 */
       m->token!=transaction.token||m->binding!=transaction.binding)return 0;
/* 进入本模块的函数实现。 */
    if(!committed) {
/* 根据运行条件选择处理分支。 */
        if(!FastPeer_Save(m->sender_id,m->binding))return 0;
/* 更新运行状态或计算结果。 */
        committed=1;Reply(now,m,LINK_PAIR_DONE);return 1;
/* 结束当前代码块。 */
    }
/* 调用相关模块接口完成当前处理步骤。 */
    Reply(now,m,LINK_PAIR_DONE);return 0;
/* 结束当前代码块。 */
}
