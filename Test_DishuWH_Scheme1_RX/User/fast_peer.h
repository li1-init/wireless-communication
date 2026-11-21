/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 根据编译条件选择对应实现。 */
#ifndef FAST_PEER_H
/* 定义编译期常量或接口宏。 */
#define FAST_PEER_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/* 引入本模块所需的头文件接口。 */
#include "link_protocol.h"
/* 执行当前语句。 */
void FastPeer_Init(void);
/* 执行当前语句。 */
uint32_t FastPeer_Id(void);
/* 执行当前语句。 */
uint32_t FastPeer_Binding(void);
/* 执行当前语句。 */
uint8_t FastPeer_Save(uint32_t peer,uint32_t binding);
/* 执行当前语句。 */
uint8_t FastPeer_Accepts(const LinkMessage *m,uint32_t local);
/* 根据编译条件选择对应实现。 */
#endif
