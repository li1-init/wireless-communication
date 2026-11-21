/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：非易失存储底层：读取 Data EEPROM，以数值和反码成对写入并校验。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 根据编译条件选择对应实现。 */
#ifndef NV_STORE_H
/* 定义编译期常量或接口宏。 */
#define NV_STORE_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/** @brief 读取指定 Data EEPROM 地址的 32 位值。 */
/* 执行当前语句。 */
uint32_t Nv_ReadWord(uint32_t address);
/** @brief 判断全零擦除值或旧版全 FF 填充值。 */
/* 执行当前语句。 */
uint8_t Nv_IsBlank(uint32_t value);
/** @brief 写入数值及反码并读回验证，最后锁定 EEPROM。 */
/* 执行当前语句。 */
uint8_t Nv_WritePair(uint32_t address,uint32_t value);
/** @brief 写入一段四字节对齐的数据并逐字节读回校验。 */
/* 执行当前语句。 */
uint8_t Nv_WriteBytes(uint32_t address,const uint8_t *data,uint16_t length);
/** @brief 从 Data EEPROM 读取任意字节区域。 */
/* 执行当前语句。 */
void Nv_ReadBytes(uint32_t address,uint8_t *data,uint16_t length);
/* 根据编译条件选择对应实现。 */
#endif
