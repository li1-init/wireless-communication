#ifndef FAST_PAIR_H
#define FAST_PAIR_H
#include "link_protocol.h"
void FastPair_Init(uint8_t transmitter,uint32_t local);
void FastPair_Begin(uint32_t now,uint32_t session);
void FastPair_Stop(void);
uint8_t FastPair_Active(void);
uint8_t FastPair_Expired(uint32_t now);
uint8_t FastPair_Output(uint32_t now,LinkMessage *out);
/* 0 rejected/no complete, 1 first RX commit, 2 TX final completion.
 * Output response is queued for a 15ms radio turnaround, never busy-waited. */
uint8_t FastPair_Input(uint32_t now,const LinkMessage *m);
#endif
