/* Dual committed EEPROM records per peer, with CRC and generation counter.
 * Persist replay high-water mark before acting on a new command. */
#include <string.h>
#include "pair_store.h"
#include "nv_store.h"
#define BASE 0x08080200UL
#define MAGIC 0x34524950UL
/* Strict one-to-one: one current peer; explicit pairing replaces it. */
static PairEntry peers[1];
static uint8_t active_slot[1];
static void Put(uint8_t *p,uint32_t v)
{ uint8_t i; for(i=0;i<4;++i) p[i]=(uint8_t)(v>>(8U*i)); }
static uint32_t Get(const uint8_t *p)
{ return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
static uint32_t Crc(const uint8_t *p)
{
    uint32_t c=~0UL;
    uint8_t i,b;
    for(i=0;i<60;++i) { c^=p[i]; for(b=0;b<8;++b) c=(c>>1)^((c&1)?0xedb88320UL:0); }
    return ~c;
}
static uint32_t Address(uint8_t peer,uint8_t slot)
{ return BASE+(uint32_t)peer*128UL+(uint32_t)slot*64UL; }
static uint8_t Read(uint8_t peer,uint8_t slot,PairEntry *e)
{
    uint8_t r[64];
    Nv_ReadBytes(Address(peer,slot),r,64);
    if(Get(r)!=MAGIC || Crc(r)!=Get(r+60) || !Get(r+4) || !Get(r+52) ||
       Get(r+48)>1U || Get(r+56)!=~Get(r+52)) return 0;
    e->id=Get(r+4); memcpy(e->key,r+8,32);
    e->last_session=Get(r+40); e->last_token=Get(r+44);
    e->seen=(uint8_t)Get(r+48); e->generation=Get(r+52); e->valid=1;
    return 1;
}
static uint8_t Save(uint8_t index,PairEntry *e)
{
    uint8_t r[64],zero[4]={0},target=(uint8_t)!active_slot[index];
    uint32_t address=Address(index,target);
    if(e->generation>=0xfffffffeUL) return 0;
    memset(r,0,sizeof(r));
    Put(r,MAGIC); Put(r+4,e->id); memcpy(r+8,e->key,32);
    Put(r+40,e->last_session); Put(r+44,e->last_token); Put(r+48,e->seen);
    Put(r+52,e->generation+1); Put(r+56,~(e->generation+1)); Put(r+60,Crc(r));
    /* Invalidate destination, write body, publish magic last. Old slot survives. */
    if(!Nv_WriteBytes(address,zero,4) || !Nv_WriteBytes(address+4,r+4,60) ||
       !Nv_WriteBytes(address,r,4)) return 0;
    ++e->generation; e->valid=1;
    active_slot[index]=target; peers[index]=*e;
    return 1;
}
void PairStore_Init(void)
{
    uint8_t i;
    memset(peers,0,sizeof(peers));
    for(i=0;i<1;++i) {
        PairEntry a={0},b={0};
        uint8_t va=Read(i,0,&a),vb=Read(i,1,&b);
        active_slot[i]=0;
        if(vb && (!va || b.generation>a.generation)) { peers[i]=b; active_slot[i]=1; }
        else if(va) peers[i]=a;
    }
}
int PairStore_Find(uint32_t id)
{ return peers[0].valid && peers[0].id==id ? 0 : -1; }
PairEntry *PairStore_Get(int slot)
{ return slot==0 && peers[0].valid ? &peers[0] : 0; }
uint8_t PairStore_GetKey(uint32_t id,uint8_t key[32])
{
    int i=PairStore_Find(id);
    if(i<0) return 0;
    memcpy(key,peers[i].key,32); return 1;
}
uint8_t PairStore_AddLink(uint32_t id,const uint8_t key[32])
{
    int i=PairStore_Find(id);
    PairEntry e={0};
    if(!id || id==0xffffffffUL) return 0;
    if(i<0)i=0; /* Replace only through the explicitly opened pairing window. */
    e=peers[i];
    if(e.valid && e.id==id && !memcmp(e.key,key,32)) return 1;
    e.id=id; memcpy(e.key,key,32); e.seen=0; e.last_session=e.last_token=0;
    return Save((uint8_t)i,&e);
}
uint8_t PairStore_AcceptData(const LinkMessage *m)
{
    int i=PairStore_Find(m->sender_id);
    PairEntry e;
    if(i<0 || m->type!=LINK_DATA) return 0;
    e=peers[i];
    if(e.seen && (m->session<e.last_session ||
       (m->session==e.last_session && m->token<=e.last_token))) return 0;
    e.seen=1; e.last_session=m->session; e.last_token=m->token;
    return Save((uint8_t)i,&e);
}
