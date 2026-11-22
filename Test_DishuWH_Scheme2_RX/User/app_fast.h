/**
 * @file app_fast.h
 * @brief 快速配对和链路应用状态接口。
 */
#ifndef APP_FAST_H
#define APP_FAST_H

#include "link_protocol.h"

/**
 * @brief 链路两端的身份及序号状态。
 */
typedef struct
{
    uint32_t local_id;
    uint32_t peer_id;
    uint32_t session;
    uint32_t sequence;
} FastIdentityState;

/**
 * @brief 链路运行计数器。
 */
typedef struct
{
    uint32_t pair_success_count;
    uint32_t pair_timeout_count;
    uint32_t rx_valid_count;
    uint32_t rx_rejected_count;
    uint32_t ack_count;
    uint32_t ack_timeout_count;
    uint32_t tx_failed;
} FastCounterState;

/**
 * @brief 链路当前状态及最近一次业务数据。
 */
typedef struct
{
    uint8_t online;
    uint8_t pair_mode;
    uint8_t last_command;
    uint8_t last_keys;
} FastRuntimeState;

/**
 * @brief 快速链路的可观察状态。
 */
typedef struct
{
    FastIdentityState identity;
    FastCounterState counters;
    FastRuntimeState runtime;
} FastStatus;

extern volatile FastStatus g_link;

/**
 * @brief 初始化板级资源、持久化会话号和无线链路。
 */
void AppFast_Init(void);

/**
 * @brief 执行一次按键、配对、无线收发和指示灯轮询。
 */
void AppFast_Poll(void);

#endif
