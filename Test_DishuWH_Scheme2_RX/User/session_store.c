/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：启动记录模块：用交替 EEPROM 记录分配启动或事务代号，检查记录完整性。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This module stores reboot epochs in STM32L1 Data EEPROM. */
/* 引入本模块所需的头文件接口。 */
#include "session_store.h"
/* 引入本模块所需的头文件接口。 */
#include "nv_store.h"
/** @brief 读取并验证启动代号及反码，只返回完整记录。 */
/* 进入本模块的函数实现。 */
static uint8_t ReadRecord(uint32_t address,uint32_t *value)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t count=Nv_ReadWord(address);
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t inverse=Nv_ReadWord(address+4UL);
/* 根据运行条件选择处理分支。 */
    if(Nv_IsBlank(count) || inverse!=~count) return 0U;
    *value=count;
/* 返回当前函数的处理结果。 */
    return 1U;
/* 结束当前代码块。 */
}
/** @brief 选择最新有效记录，在另一槽写入下一代号；成功后才返回。 */
/* 进入本模块的函数实现。 */
uint8_t SessionStore_Next(uint32_t *session)
/* 开始当前代码块。 */
{
/* 定义或初始化本模块使用的状态数据。 */
    const uint32_t first=0x08080008UL,second=0x08080010UL;
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t a=0U,b=0U,legacy,previous=0U,next,address;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t valid_a=ReadRecord(first,&a),valid_b=ReadRecord(second,&b);
/* 进入本模块的函数实现。 */
    if(valid_a && (!valid_b || a>b)) {
/* 更新运行状态或计算结果。 */
        previous=a; address=second;
/* 说明本行代码在当前模块中的处理作用。 */
    } else if(valid_b) {
/* 更新运行状态或计算结果。 */
        previous=b; address=first;
/* 说明本行代码在当前模块中的处理作用。 */
    } else {
/* 更新运行状态或计算结果。 */
        legacy=Nv_ReadWord(0x08080000UL);
/* 根据运行条件选择处理分支。 */
        if(!Nv_IsBlank(Nv_ReadWord(first+4UL)) ||
/* 说明本行代码在当前模块中的处理作用。 */
           !Nv_IsBlank(Nv_ReadWord(second)) ||
/* 执行当前语句。 */
           !Nv_IsBlank(Nv_ReadWord(second+4UL))) return 0U;
/* 更新运行状态或计算结果。 */
        previous=Nv_IsBlank(legacy) ? 0U : legacy;
/* 更新运行状态或计算结果。 */
        address=first;
/* 结束当前代码块。 */
    }
/* 根据运行条件选择处理分支。 */
    if(previous>=0xFFFFFFFEUL) return 0U;
/* 更新运行状态或计算结果。 */
    next=previous+1UL;
/* 根据运行条件选择处理分支。 */
    if(!Nv_WritePair(address,next) || !ReadRecord(address,&a) || a!=next) return 0U;
    *session=next;
/* 返回当前函数的处理结果。 */
    return 1U;
/* 结束当前代码块。 */
}
