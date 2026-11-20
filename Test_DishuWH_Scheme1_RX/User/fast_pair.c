#include <string.h>
#include "fast_pair.h"
#include "fast_peer.h"
static uint8_t tx,active,stage,pending,committed,candidate_valid;
static uint32_t local,started,last_send,reply_at,candidate_at;
static LinkMessage transaction,queued;
void FastPair_Init(uint8_t transmitter,uint32_t id)
{tx=transmitter;local=id;active=stage=pending=committed=candidate_valid=0;}
void FastPair_Stop(void){active=pending=candidate_valid=committed=0;}
void FastPair_Begin(uint32_t now,uint32_t session)
{
    FastPair_Stop();active=1;started=now;last_send=now-150UL;stage=0;
    memset(&transaction,0,sizeof(transaction));
    transaction.sender_id=local;transaction.session=session;transaction.token=1;
    transaction.type=LINK_PAIR_REQUEST;
}
uint8_t FastPair_Active(void){return active;}
uint8_t FastPair_Expired(uint32_t now)
{
    if(tx&&active&&(uint32_t)(now-started)>=10000UL){FastPair_Stop();return 1;}
    return 0;
}
static void Reply(uint32_t now,const LinkMessage *m,uint8_t type)
{
    queued=*m;queued.type=type;queued.sender_id=local;queued.receiver_id=m->sender_id;
    queued.payload=1;pending=1;reply_at=now;
}
uint8_t FastPair_Output(uint32_t now,LinkMessage *out)
{
    if(!active)return 0;
    if(pending&&(uint32_t)(now-reply_at)>=15UL){*out=queued;pending=0;return 1;}
    if(tx&&(uint32_t)(now-last_send)>=150UL) {
        last_send=now;*out=transaction;
        if(stage){out->type=LINK_PAIR_COMMIT;out->payload=1;}
        return 1;
    }
    return 0;
}
uint8_t FastPair_Input(uint32_t now,const LinkMessage *m)
{
    if(!active||m->sender_id==local)return 0;
    if(tx) {
        if(m->receiver_id!=local||m->session!=transaction.session||
           m->token!=transaction.token)return 0;
        if(!stage&&m->type==LINK_PAIR_ACK&&m->binding==transaction.session) {
            transaction.receiver_id=m->sender_id;transaction.binding=m->binding;
            stage=1;last_send=now-150UL;
        } else if(stage&&m->type==LINK_PAIR_DONE&&
                  m->sender_id==transaction.receiver_id&&m->binding==transaction.binding) {
            if(!FastPeer_Save(m->sender_id,m->binding))return 0;
            FastPair_Stop();return 2;
        }
        return 0;
    }
    if(m->type==LINK_PAIR_REQUEST) {
        /* First candidate wins until timeout or commit; no cross-device overwrite mid-handshake. */
        if(candidate_valid&&(uint32_t)(now-candidate_at)<2000UL&&
           (m->sender_id!=transaction.sender_id||m->session!=transaction.session||
            m->token!=transaction.token))return 0;
        if(!candidate_valid||m->sender_id!=transaction.sender_id||
           m->session!=transaction.session||m->token!=transaction.token)committed=0;
        transaction=*m;transaction.binding=m->session;
        candidate_valid=1;candidate_at=now;
        Reply(now,&transaction,LINK_PAIR_ACK);return 0;
    }
    if(m->type!=LINK_PAIR_COMMIT||!candidate_valid||m->receiver_id!=local||
       m->sender_id!=transaction.sender_id||m->session!=transaction.session||
       m->token!=transaction.token||m->binding!=transaction.binding)return 0;
    if(!committed) {
        if(!FastPeer_Save(m->sender_id,m->binding))return 0;
        committed=1;Reply(now,m,LINK_PAIR_DONE);return 1;
    }
    Reply(now,m,LINK_PAIR_DONE);return 0;
}
