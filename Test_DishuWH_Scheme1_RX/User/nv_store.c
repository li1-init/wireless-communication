/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：非易失存储底层：读取 Data EEPROM，以数值和反码成对写入并校验。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 引入本模块所需的头文件接口。 */
#include "nv_store.h"
/* 引入本模块所需的头文件接口。 */
#include "stm32l1xx_flash.h"
/** @brief 读取指定 Data EEPROM 地址的 32 位值。 */
/* 进入本模块的函数实现。 */
uint32_t Nv_ReadWord(uint32_t address)
/* 开始当前代码块。 */
{
/* 返回当前函数的处理结果。 */
    return *(volatile const uint32_t *)address;
/* 结束当前代码块。 */
}
/** @brief 判断全零擦除值或旧版全 FF 填充值。 */
/* 进入本模块的函数实现。 */
uint8_t Nv_IsBlank(uint32_t value)
/* 开始当前代码块。 */
{
/* 返回当前函数的处理结果。 */
    return (value==0U || value==0xFFFFFFFFUL) ? 1U : 0U;
/* 结束当前代码块。 */
}
/** @brief 写入数值及反码并读回验证，最后锁定 EEPROM。 */
/* 进入本模块的函数实现。 */
uint8_t Nv_WritePair(uint32_t address,uint32_t value)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t ok;
/* 调用相关模块接口完成当前处理步骤。 */
    DATA_EEPROM_Unlock();
/* 更新运行状态或计算结果。 */
    ok=(uint8_t)(DATA_EEPROM_ProgramWord(address,value)==FLASH_COMPLETE &&
/* 调用相关模块接口完成当前处理步骤。 */
                 DATA_EEPROM_ProgramWord(address+4UL,~value)==FLASH_COMPLETE &&
/* 调用相关模块接口完成当前处理步骤。 */
                 Nv_ReadWord(address)==value && Nv_ReadWord(address+4UL)==~value);
/* 调用相关模块接口完成当前处理步骤。 */
    DATA_EEPROM_Lock();
/* 返回当前函数的处理结果。 */
    return ok;
/* 结束当前代码块。 */
}
/** @brief 写入四字节对齐数据，跳过无需变化的空白字并读回校验。 */
/* 进入本模块的函数实现。 */
uint8_t Nv_WriteBytes(uint32_t address,const uint8_t *data,uint16_t length)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t pos;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t ok=1U;
/* 根据运行条件选择处理分支。 */
    if((address&3U) || (length&3U) || address<0x08080000UL ||
/* 执行当前语句。 */
       address+length>0x08081000UL) return 0U;
/* 调用相关模块接口完成当前处理步骤。 */
    DATA_EEPROM_Unlock();
/* 循环处理当前任务，直到满足退出条件。 */
    for(pos=0;pos<length;pos=(uint16_t)(pos+4U)) {
/* 定义或初始化本模块使用的状态数据。 */
        uint32_t value=(uint32_t)data[pos] |
/* 说明本行代码在当前模块中的处理作用。 */
            ((uint32_t)data[pos+1U]<<8U) |
/* 说明本行代码在当前模块中的处理作用。 */
            ((uint32_t)data[pos+2U]<<16U) |
/* 执行当前语句。 */
            ((uint32_t)data[pos+3U]<<24U);
/* 根据运行条件选择处理分支。 */
        if(*(volatile const uint32_t *)(address+pos)!=value &&
/* 进入本模块的函数实现。 */
           DATA_EEPROM_ProgramWord(address+pos,value)!=FLASH_COMPLETE) {
/* 更新运行状态或计算结果。 */
            ok=0U;
/* 控制当前循环或分支的执行流程。 */
            break;
/* 结束当前代码块。 */
        }
/* 结束当前代码块。 */
    }
/* 调用相关模块接口完成当前处理步骤。 */
    DATA_EEPROM_Lock();
/* 根据运行条件选择处理分支。 */
    if(ok) for(pos=0;pos<length;++pos)
/* 根据运行条件选择处理分支。 */
        if(((volatile const uint8_t *)(address))[pos]!=data[pos]) return 0U;
/* 返回当前函数的处理结果。 */
    return ok;
/* 结束当前代码块。 */
}
/** @brief 从 Data EEPROM 逐字节读取。 */
/* 进入本模块的函数实现。 */
void Nv_ReadBytes(uint32_t address,uint8_t *data,uint16_t length)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t i;
/* 循环处理当前任务，直到满足退出条件。 */
    for(i=0;i<length;++i) data[i]=((volatile const uint8_t *)address)[i];
/* 结束当前代码块。 */
}
