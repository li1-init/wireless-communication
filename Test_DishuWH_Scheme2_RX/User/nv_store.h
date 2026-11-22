/**
 * @file nv_store.h
 * @brief Data EEPROM 读写接口。
 */
#ifndef NV_STORE_H
#define NV_STORE_H

#include <stdint.h>

/**
 * @brief 读取指定 Data EEPROM 地址的 32 位值。
 * @param address EEPROM 地址。
 * @return 读取到的 32 位数据。
 */
uint32_t Nv_ReadWord(uint32_t address);

/**
 * @brief 判断数值是否表示空白 EEPROM。
 * @param value 待读取的数值。
 * @return 表示空白时返回 1，否则返回 0。
 */
uint8_t Nv_IsBlank(uint32_t value);

/**
 * @brief 写入数值及其反码并读回验证。
 * @param address EEPROM 地址。
 * @param value 要写入的数值。
 * @return 写入并校验成功时返回 1，否则返回 0。
 */
uint8_t Nv_WritePair(uint32_t address, uint32_t value);

/**
 * @brief 写入一段数据并逐字节读回校验。
 * @param address EEPROM 起始地址。
 * @param data 待写入的数据。
 * @param length 数据长度。
 * @return 写入并校验成功时返回 1，否则返回 0。
 */
uint8_t Nv_WriteBytes(uint32_t address,
                      const uint8_t *data,
                      uint16_t length);

/**
 * @brief 从 Data EEPROM 读取任意字节区域。
 * @param address EEPROM 起始地址。
 * @param data 输出缓冲区。
 * @param length 读取长度。
 */
void Nv_ReadBytes(uint32_t address, uint8_t *data, uint16_t length);

#endif
