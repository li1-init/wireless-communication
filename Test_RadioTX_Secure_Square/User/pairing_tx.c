#include <string.h>
#include "pairing_tx.h"
#include "tx_peer.h"
extern void randombytes(unsigned char *,unsigned long long);
static uint8_t active,stage,public_key[32],secret_key[32],challenge[8],key[32],commit_proof[8];
static uint32_t started,last_request,peer;
static uint8_t cached_commit[LINK_PACKET_SIZE];
static LinkMessage request;
static uint8_t prepared;
void PairingTx_Init(void) { active=stage=prepared=0; }
void PairingTx_Prepare(void)
{
    if(active || prepared) return;
    LinkCrypto_Keypair(public_key,secret_key);
    prepared=1;
}
void PairingTx_Begin(uint32_t now,uint32_t id,uint32_t session,uint32_t token)
{
    if(!prepared) LinkCrypto_Keypair(public_key,secret_key);
    prepared=0; /* A retry is a fresh transaction, never reuse the private key. */
    randombytes(challenge,8);
    request.type=LINK_PAIR_REQUEST; request.payload=0;
    request.sender_id=id; request.receiver_id=0; request.session=session; request.token=token;
    /* Deadline and indicator share the entry timestamp, including preparation. */
    started=now; last_request=now-LINK_PAIR_INTERVAL_MS; active=1; stage=0;
}
uint8_t PairingTx_Active(void) { return active; }
uint8_t PairingTx_Expire(uint32_t now)
{
    if(active && (uint32_t)(now-started)>=LINK_PAIR_TIMEOUT_MS) { active=0; return 1; }
    return 0;
}
uint8_t PairingTx_Request(uint32_t now,uint8_t packet[LINK_PACKET_SIZE])
{
    if(!active || (uint32_t)(now-last_request)<LINK_PAIR_INTERVAL_MS) return 0;
    last_request=now;
    if(!stage) Link_EncodePairRequest(packet,&request,public_key,challenge);
    else {
        memcpy(packet,cached_commit,LINK_PACKET_SIZE);
    }
    return 1;
}
uint8_t PairingTx_AcceptAck(uint32_t now,const uint8_t packet[LINK_PACKET_SIZE])
{
    LinkPair ack;
    LinkMessage done;
    uint8_t proof[8],difference=0,i,candidate[32];
    if(!active || (uint32_t)(now-started)>=LINK_PAIR_TIMEOUT_MS) return 0;
    if(!stage) {
        if(!Link_DecodePairAck(packet,&ack) ||
           ack.message.receiver_id!=request.sender_id ||
           ack.message.session!=request.session || ack.message.token!=request.token) return 0;
        if(!LinkCrypto_Shared(candidate,ack.public_key,secret_key)) return 0;
        LinkCrypto_Proof(proof,candidate,request.sender_id,ack.message.sender_id,
                         request.session,request.token,public_key,ack.public_key,challenge);
        for(i=0;i<8;++i) difference|=(uint8_t)(proof[i]^ack.extra[i]);
        if(difference) return 0;
        LinkCrypto_Proof(commit_proof,candidate,ack.message.sender_id,request.sender_id,
                         request.session,request.token,ack.public_key,public_key,challenge);
        memcpy(key,candidate,32); peer=ack.message.sender_id; stage=1;
        {
            LinkMessage commit=request;
            commit.receiver_id=peer; commit.type=LINK_PAIR_COMMIT; commit.payload=LINK_PAIR_ACK_OK;
            memcpy(commit.proof,commit_proof,8);
            Link_EncodeData(cached_commit,&commit,key);
        }
        last_request=now-LINK_PAIR_INTERVAL_MS;
        return 0;
    }
    if(!Link_DecodeData(packet,&done,key) || done.type!=LINK_PAIR_DONE ||
       done.sender_id!=peer || done.receiver_id!=request.sender_id ||
       done.session!=request.session || done.token!=request.token) return 0;
    if(!TxPeer_Save(peer,key)) return 0;
    active=0; memset(secret_key,0,32); return 1;
}
void PairingTx_Abort(void) { active=0; memset(secret_key,0,32); }
