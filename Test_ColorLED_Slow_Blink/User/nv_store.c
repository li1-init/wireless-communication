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
/** @brief 写入四字节对齐数据，跳过无需变化的空白字并读回校验。 */
uint8_t Nv_WriteBytes(uint32_t address,const uint8_t *data,uint16_t length)
{
    uint16_t pos;
    uint8_t ok=1U;
    if((address&3U) || (length&3U) || address<0x08080000UL ||
       address+length>0x08081000UL) return 0U;
    DATA_EEPROM_Unlock();
    for(pos=0;pos<length;pos=(uint16_t)(pos+4U)) {
        uint32_t value=(uint32_t)data[pos] |
            ((uint32_t)data[pos+1U]<<8U) |
            ((uint32_t)data[pos+2U]<<16U) |
            ((uint32_t)data[pos+3U]<<24U);
        if(*(volatile const uint32_t *)(address+pos)!=value &&
           DATA_EEPROM_ProgramWord(address+pos,value)!=FLASH_COMPLETE) {
            ok=0U;
            break;
        }
    }
    DATA_EEPROM_Lock();
    if(ok) for(pos=0;pos<length;++pos)
        if(((volatile const uint8_t *)(address))[pos]!=data[pos]) return 0U;
    return ok;
}
/** @brief 从 Data EEPROM 逐字节读取。 */
void Nv_ReadBytes(uint32_t address,uint8_t *data,uint16_t length)
{
    uint16_t i;
    for(i=0;i<length;++i) data[i]=((volatile const uint8_t *)address)[i];
}
