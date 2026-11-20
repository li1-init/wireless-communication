#ifndef FAST_PEER_H
#define FAST_PEER_H
#include <stdint.h>
#include "link_protocol.h"
void FastPeer_Init(void);
uint32_t FastPeer_Id(void);
uint32_t FastPeer_Binding(void);
uint8_t FastPeer_Save(uint32_t peer,uint32_t binding);
uint8_t FastPeer_Accepts(const LinkMessage *m,uint32_t local);
#endif
