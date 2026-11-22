/**
 * @file fast_pair.h
 * @brief 快速配对状态机接口。
 */
#ifndef FAST_PAIR_H
#define FAST_PAIR_H

#include "link_protocol.h"

/**
 * @brief 初始化快速配对状态机。
 * @param transmitter 是否为发送端。
 * @param local 本机设备 ID。
 */
void FastPair_Init(uint8_t transmitter, uint32_t local);

/**
 * @brief 开始一次快速配对事务。
 * @param now 当前系统毫秒时间戳。
 * @param session 本次配对使用的会话号。
 */
void FastPair_Begin(uint32_t now, uint32_t session);

/**
 * @brief 停止当前快速配对事务。
 */
void FastPair_Stop(void);

/**
 * @brief 查询快速配对是否处于活动状态。
 * @return 活动时返回 1，否则返回 0。
 */
uint8_t FastPair_Active(void);

/**
 * @brief 检查发送端配对事务是否超时。
 * @param now 当前系统毫秒时间戳。
 * @return 已超时时返回 1，否则返回 0。
 */
uint8_t FastPair_Expired(uint32_t now);

/**
 * @brief 根据当前阶段生成待发送的配对消息。
 * @param now 当前系统毫秒时间戳。
 * @param out 输出的配对消息。
 * @return 需要发送消息时返回 1，否则返回 0。
 */
uint8_t FastPair_Output(uint32_t now, LinkMessage *out);

/**
 * @brief 输入一帧配对消息并推进状态机。
 * @param now 当前系统毫秒时间戳。
 * @param message 收到的配对消息。
 * @return 0 表示拒绝或未完成，1 表示接收端首次提交，2 表示发送端最终完成。
 */
uint8_t FastPair_Input(uint32_t now, const LinkMessage *message);

#endif
