/* 文件说明：方板应用：处理按键、对码、发送数据和确认回包。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This file declares the square transmitter application interface. */
#ifndef APP_TX_H
#define APP_TX_H
#include "link_protocol.h"
typedef struct {
    uint32_t role,local_id,sequence,session,tx_failed;
    uint32_t rx_valid_count,rx_rejected_count,last_sender_id,last_sequence,last_valid_ms;
    uint8_t last_command,last_keys,last_packet[LINK_PACKET_SIZE];
    uint8_t online,sender_locked,reserved[3];
    uint32_t pair_success_count,pair_timeout_count;
    uint8_t pair_mode;
    uint32_t ack_timeout_count; /* 未收到普通数据确认；不属于硬件故障。 */
} LinkAppStatus;
extern volatile LinkAppStatus g_link;
/** @brief 初始化方板、启动代号、按键、灯和无线模块。 */
void AppTx_Init(void);
/** @brief 执行一轮方板业务：按键、对码、普通发送以及确认结果处理。 */
void AppTx_Poll(void);
#endif
