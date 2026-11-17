/* 文件说明：圆板应用：处理 SW 长按、对码保存、数据接收和确认回包。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This file declares the round receiver application interface. */
#ifndef APP_RX_H
#define APP_RX_H
#include "link_protocol.h"
typedef struct {
    uint32_t role,local_id,sequence,session,tx_failed;
    uint32_t rx_valid_count,rx_rejected_count,last_sender_id,last_sequence,last_valid_ms;
    uint8_t last_command,last_keys,last_packet[LINK_PACKET_SIZE];
    uint8_t online,sender_locked,reserved[3];
    uint32_t pair_success_count,pair_ack_count;
    uint8_t pair_mode;
} LinkAppStatus;
extern volatile LinkAppStatus g_link;
/** @brief 初始化圆板、读取配对名单并启动无线接收。 */
void AppRx_Init(void);
/** @brief 执行一轮圆板业务：长按切换、解包、反馈和确认回包。 */
void AppRx_Poll(void);
#endif
