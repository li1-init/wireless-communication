#ifndef TX_PEER_H
#define TX_PEER_H
#include <stdint.h>
void TxPeer_Init(void);
uint32_t TxPeer_Id(void);
const uint8_t *TxPeer_Key(void);
uint8_t TxPeer_Save(uint32_t id,const uint8_t key[32]);
#endif
