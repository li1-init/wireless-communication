/* 文件说明：非易失存储底层：读取 Data EEPROM，以数值和反码成对写入并校验。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#include "nv_store.h"
#include "stm32l1xx_flash.h"
/** @brief 读取指定 Data EEPROM 地址的 32 位值。 */
uint32_t Nv_ReadWord(uint32_t address)
{
    return *(volatile const uint32_t *)address;
}
/** @brief 判断全零擦除值或旧版全 FF 填充值。 */
uint8_t Nv_IsBlank(uint32_t value)
{
    return (value==0U || value==0xFFFFFFFFUL) ? 1U : 0U;
}
/** @brief 写入数值及反码并读回验证，最后锁定 EEPROM。 */
uint8_t Nv_WritePair(uint32_t address,uint32_t value)
{
    uint8_t ok;
    DATA_EEPROM_Unlock();
    ok=(uint8_t)(DATA_EEPROM_ProgramWord(address,value)==FLASH_COMPLETE &&
                 DATA_EEPROM_ProgramWord(address+4UL,~value)==FLASH_COMPLETE &&
                 Nv_ReadWord(address)==value && Nv_ReadWord(address+4UL)==~value);
    DATA_EEPROM_Lock();
    return ok;
}
