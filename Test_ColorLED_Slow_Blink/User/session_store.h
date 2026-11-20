/* 文件说明：启动记录模块：用交替 EEPROM 记录分配启动或事务代号，检查记录完整性。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* This file declares persistent transmitter session allocation. */
#ifndef SESSION_STORE_H
#define SESSION_STORE_H
#include <stdint.h>
/** @brief 选择最新有效记录，在另一槽写入下一代号；成功后才返回。 */
uint8_t SessionStore_Next(uint32_t *session);
#endif
