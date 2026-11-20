/* 文件说明：启动记录模块：用交替 EEPROM 记录分配启动或事务代号，检查记录完整性。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This module stores reboot epochs in STM32L1 Data EEPROM. */
#include "session_store.h"
#include "nv_store.h"
/** @brief 读取并验证启动代号及反码，只返回完整记录。 */
static uint8_t ReadRecord(uint32_t address,uint32_t *value)
{
    uint32_t count=Nv_ReadWord(address);
    uint32_t inverse=Nv_ReadWord(address+4UL);
    if(Nv_IsBlank(count) || inverse!=~count) return 0U;
    *value=count;
    return 1U;
}
/** @brief 选择最新有效记录，在另一槽写入下一代号；成功后才返回。 */
uint8_t SessionStore_Next(uint32_t *session)
{
    const uint32_t first=0x08080008UL,second=0x08080010UL;
    uint32_t a=0U,b=0U,legacy,previous=0U,next,address;
    uint8_t valid_a=ReadRecord(first,&a),valid_b=ReadRecord(second,&b);
    if(valid_a && (!valid_b || a>b)) {
        previous=a; address=second;
    } else if(valid_b) {
        previous=b; address=first;
    } else {
        legacy=Nv_ReadWord(0x08080000UL);
        if(!Nv_IsBlank(Nv_ReadWord(first+4UL)) ||
           !Nv_IsBlank(Nv_ReadWord(second)) ||
           !Nv_IsBlank(Nv_ReadWord(second+4UL))) return 0U;
        previous=Nv_IsBlank(legacy) ? 0U : legacy;
        address=first;
    }
    if(previous>=0xFFFFFFFEUL) return 0U;
    next=previous+1UL;
    if(!Nv_WritePair(address,next) || !ReadRecord(address,&a) || a!=next) return 0U;
    *session=next;
    return 1U;
}
