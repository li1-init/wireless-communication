/* 文件说明：非易失存储底层：读取 Data EEPROM，以数值和反码成对写入并校验。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef NV_STORE_H
#define NV_STORE_H
#include <stdint.h>
/** @brief 读取指定 Data EEPROM 地址的 32 位值。 */
uint32_t Nv_ReadWord(uint32_t address);
/** @brief 判断全零擦除值或旧版全 FF 填充值。 */
uint8_t Nv_IsBlank(uint32_t value);
/** @brief 写入数值及反码并读回验证，最后锁定 EEPROM。 */
uint8_t Nv_WritePair(uint32_t address,uint32_t value);
#endif
