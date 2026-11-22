/**
 * @file fast_peer.h
 * @brief 已配对对端信息接口。
 */
#ifndef FAST_PEER_H
#define FAST_PEER_H

#include <stdint.h>
#include "link_protocol.h"

/**
 * @brief 从非易失存储加载已绑定的对端信息。
 */
void FastPeer_Init(void);

/**
 * @brief 获取已绑定的对端设备 ID。
 * @return 对端设备 ID，未绑定时返回 0。
 */
uint32_t FastPeer_Id(void);

/**
 * @brief 获取当前绑定标识。
 * @return 绑定标识，未绑定时返回 0。
 */
uint32_t FastPeer_Binding(void);

/**
 * @brief 保存对端设备 ID 和绑定标识。
 * @param peer 对端设备 ID。
 * @param binding 绑定标识。
 * @return 保存并校验成功时返回 1，否则返回 0。
 */
uint8_t FastPeer_Save(uint32_t peer, uint32_t binding);

/**
 * @brief 判断收到的消息是否来自当前绑定对端。
 * @param message 待检查的消息。
 * @param local 本机设备 ID。
 * @return 消息可接受时返回 1，否则返回 0。
 */
uint8_t FastPeer_Accepts(const LinkMessage *message, uint32_t local);

#endif
