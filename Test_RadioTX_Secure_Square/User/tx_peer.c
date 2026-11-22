#include <string.h>
#include "tx_peer.h"
#include "pair_store.h"
#include "nv_store.h"
static uint32_t peer_id;
static uint8_t peer_key[32];
void TxPeer_Init(void)
{
    PairEntry *entry;
    PairStore_Init();peer_id=0;memset(peer_key,0,32);
    entry=PairStore_Get(0);
    if(entry){peer_id=entry->id;memcpy(peer_key,entry->key,32);}
}
uint32_t TxPeer_Id(void) { return peer_id; }
const uint8_t *TxPeer_Key(void) { return peer_key; }
uint8_t TxPeer_Save(uint32_t id,const uint8_t key[32])
{
    if(!PairStore_AddLink(id,key))return 0;
    peer_id=id; memcpy(peer_key,key,32); return 1;
}
