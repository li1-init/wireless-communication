/**
 * @file nv_store.c
 * @brief Data EEPROM 读写实现。
 */
#include "nv_store.h"
#include "stm32l1xx_flash.h"

/**
 * @brief 读取指定 Data EEPROM 地址的 32 位值。
 * @param address EEPROM 地址。
 * @return 读取到的 32 位数据。
 */
uint32_t Nv_ReadWord(uint32_t address)
{
    return *(volatile const uint32_t *)address;
}

/**
 * @brief 判断数值是否表示空白 EEPROM。
 * @param value 待读取的数值。
 * @return 表示空白时返回 1，否则返回 0。
 */
uint8_t Nv_IsBlank(uint32_t value)
{
    return (value == 0U || value == 0xFFFFFFFFUL) ? 1U : 0U;
}

/**
 * @brief 写入数值及其反码并读回验证。
 * @param address EEPROM 地址。
 * @param value 要写入的数值。
 * @return 写入并校验成功时返回 1，否则返回 0。
 */
uint8_t Nv_WritePair(uint32_t address, uint32_t value)
{
    uint8_t ok;

    DATA_EEPROM_Unlock();
    ok = (uint8_t)(
        DATA_EEPROM_ProgramWord(address, value) == FLASH_COMPLETE &&
        DATA_EEPROM_ProgramWord(address + 4UL, ~value) == FLASH_COMPLETE &&
        Nv_ReadWord(address) == value &&
        Nv_ReadWord(address + 4UL) == ~value);
    DATA_EEPROM_Lock();

    return ok;
}

/**
 * @brief 写入一段数据并逐字节读回校验。
 * @param address EEPROM 起始地址。
 * @param data 待写入的数据。
 * @param length 数据长度。
 * @return 写入并校验成功时返回 1，否则返回 0。
 */
uint8_t Nv_WriteBytes(uint32_t address,
                      const uint8_t *data,
                      uint16_t length)
{
    uint16_t position;
    uint32_t value;
    uint8_t ok;

    position = 0U;
    value = 0U;
    ok = 1U;

    if ((address & 3U) ||
        (length & 3U) ||
        address < 0x08080000UL ||
        address + length > 0x08081000UL)
    {
        return 0U;
    }

    DATA_EEPROM_Unlock();

    for (position = 0U;
         position < length;
         position = (uint16_t)(position + 4U))
    {
        value = (uint32_t)data[position] |
                ((uint32_t)data[position + 1U] << 8U) |
                ((uint32_t)data[position + 2U] << 16U) |
                ((uint32_t)data[position + 3U] << 24U);

        if (*(volatile const uint32_t *)(address + position) != value &&
            DATA_EEPROM_ProgramWord(address + position, value) != FLASH_COMPLETE)
        {
            ok = 0U;
            break;
        }
    }

    DATA_EEPROM_Lock();

    if (ok)
    {
        for (position = 0U; position < length; ++position)
        {
            if (((volatile const uint8_t *)address)[position] != data[position])
            {
                return 0U;
            }
        }
    }

    return ok;
}

/**
 * @brief 从 Data EEPROM 读取任意字节区域。
 * @param address EEPROM 起始地址。
 * @param data 输出缓冲区。
 * @param length 读取长度。
 */
void Nv_ReadBytes(uint32_t address, uint8_t *data, uint16_t length)
{
    uint16_t index;

    for (index = 0U; index < length; ++index)
    {
        data[index] = ((volatile const uint8_t *)address)[index];
    }
}
