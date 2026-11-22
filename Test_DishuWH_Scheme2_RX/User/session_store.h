/**
 * @file session_store.h
 * @brief 持久化会话号分配接口。
 */
#ifndef SESSION_STORE_H
#define SESSION_STORE_H

#include <stdint.h>

/**
 * @brief 选择最新记录并写入下一个会话号。
 * @param session 输出的会话号。
 * @return 分配并校验成功时返回 1，否则返回 0。
 */
uint8_t SessionStore_Next(uint32_t *session);

#endif
