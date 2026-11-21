/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：启动记录模块：用交替 EEPROM 记录分配启动或事务代号，检查记录完整性。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This file declares persistent transmitter session allocation. */
/* 根据编译条件选择对应实现。 */
#ifndef SESSION_STORE_H
/* 定义编译期常量或接口宏。 */
#define SESSION_STORE_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/** @brief 选择最新有效记录，在另一槽写入下一代号；成功后才返回。 */
/* 执行当前语句。 */
uint8_t SessionStore_Next(uint32_t *session);
/* 根据编译条件选择对应实现。 */
#endif
