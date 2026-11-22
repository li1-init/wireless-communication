#ifndef PAIRING_TX_H
#define PAIRING_TX_H
#include "link_protocol.h"
#define PAIR_TX_TIMEOUT_MS LINK_PAIR_TIMEOUT_MS
void PairingTx_Init(void);
/* Prepare a single-use ephemeral key at boot, outside the button handshake. */
void PairingTx_Prepare(void);
void PairingTx_Begin(uint32_t now,uint32_t id,uint32_t session,uint32_t token);
uint8_t PairingTx_Active(void);
uint8_t PairingTx_Expire(uint32_t now);
uint8_t PairingTx_Request(uint32_t now,uint8_t packet[LINK_PACKET_SIZE]);
uint8_t PairingTx_AcceptAck(uint32_t now,const uint8_t packet[LINK_PACKET_SIZE]);
void PairingTx_Abort(void);
#endif
