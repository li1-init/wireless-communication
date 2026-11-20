#ifndef APP_FAST_H
#define APP_FAST_H
#include "link_protocol.h"
typedef struct {
    uint32_t local_id,peer_id,session,sequence;
    uint32_t pair_success_count,pair_timeout_count,rx_valid_count,rx_rejected_count;
    uint32_t ack_count,ack_timeout_count,tx_failed;
    uint8_t online,pair_mode,last_command,last_keys;
} FastStatus;
extern volatile FastStatus g_link;
void AppFast_Init(void);
void AppFast_Poll(void);
#endif
