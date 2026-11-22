/**
 * @file session_store.c
 * @brief 持久化会话号分配实现。
 */
#include "session_store.h"
#include "nv_store.h"

#define SESSION_STORE_FIRST 0x08080008UL
#define SESSION_STORE_SECOND 0x08080010UL
#define SESSION_STORE_LEGACY 0x08080000UL

/**
 * @brief 读取并验证一条会话号记录。
 * @param address 记录起始地址。
 * @param value 输出的会话号。
 * @return 记录有效时返回 1，否则返回 0。
 */
static uint8_t ReadRecord(uint32_t address, uint32_t *value)
{
    uint32_t count;
    uint32_t inverse;

    count = Nv_ReadWord(address);
    inverse = Nv_ReadWord(address + 4UL);

    if (Nv_IsBlank(count) || inverse != ~count)
    {
        return 0U;
    }

    *value = count;
    return 1U;
}

/**
 * @brief 选择最新记录并写入下一个会话号。
 * @param session 输出的会话号。
 * @return 分配并校验成功时返回 1，否则返回 0。
 */
uint8_t SessionStore_Next(uint32_t *session)
{
    uint32_t first;
    uint32_t second;
    uint32_t legacy;
    uint32_t previous;
    uint32_t next;
    uint32_t address;
    uint8_t first_valid;
    uint8_t second_valid;

    first = 0U;
    second = 0U;
    legacy = 0U;
    previous = 0U;
    next = 0U;
    address = 0U;
    first_valid = ReadRecord(SESSION_STORE_FIRST, &first);
    second_valid = ReadRecord(SESSION_STORE_SECOND, &second);

    if (first_valid && (!second_valid || first > second))
    {
        previous = first;
        address = SESSION_STORE_SECOND;
    }
    else if (second_valid)
    {
        previous = second;
        address = SESSION_STORE_FIRST;
    }
    else
    {
        legacy = Nv_ReadWord(SESSION_STORE_LEGACY);

        if (!Nv_IsBlank(Nv_ReadWord(SESSION_STORE_FIRST + 4UL)) ||
            !Nv_IsBlank(Nv_ReadWord(SESSION_STORE_SECOND)) ||
            !Nv_IsBlank(Nv_ReadWord(SESSION_STORE_SECOND + 4UL)))
        {
            return 0U;
        }

        previous = Nv_IsBlank(legacy) ? 0U : legacy;
        address = SESSION_STORE_FIRST;
    }

    if (previous >= 0xFFFFFFFEUL)
    {
        return 0U;
    }

    next = previous + 1UL;

    if (!Nv_WritePair(address, next) ||
        !ReadRecord(address, &first) ||
        first != next)
    {
        return 0U;
    }

    *session = next;
    return 1U;
}
