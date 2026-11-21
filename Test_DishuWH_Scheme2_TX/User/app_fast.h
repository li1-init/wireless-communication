/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 根据编译条件选择对应实现。 */
#ifndef APP_FAST_H
/* 定义编译期常量或接口宏。 */
#define APP_FAST_H
/* 引入本模块所需的头文件接口。 */
#include "link_protocol.h"
/* 定义本模块使用的数据类型。 */
typedef struct {
/* 执行当前语句。 */
    uint32_t local_id,peer_id,session,sequence;
/* 执行当前语句。 */
    uint32_t pair_success_count,pair_timeout_count,rx_valid_count,rx_rejected_count;
/* 执行当前语句。 */
    uint32_t ack_count,ack_timeout_count,tx_failed;
/* 执行当前语句。 */
    uint8_t online,pair_mode,last_command,last_keys;
/* 执行当前语句。 */
} FastStatus;
/* 定义或初始化本模块使用的状态数据。 */
extern volatile FastStatus g_link;
/* 执行当前语句。 */
void AppFast_Init(void);
/* 执行当前语句。 */
void AppFast_Poll(void);
/* 根据编译条件选择对应实现。 */
#endif
