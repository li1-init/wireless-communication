/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 根据编译条件选择对应实现。 */
#ifndef FAST_PAIR_H
/* 定义编译期常量或接口宏。 */
#define FAST_PAIR_H
/* 引入本模块所需的头文件接口。 */
#include "link_protocol.h"
/* 执行当前语句。 */
void FastPair_Init(uint8_t transmitter,uint32_t local);
/* 执行当前语句。 */
void FastPair_Begin(uint32_t now,uint32_t session);
/* 执行当前语句。 */
void FastPair_Stop(void);
/* 执行当前语句。 */
uint8_t FastPair_Active(void);
/* 执行当前语句。 */
uint8_t FastPair_Expired(uint32_t now);
/* 执行当前语句。 */
uint8_t FastPair_Output(uint32_t now,LinkMessage *out);
/* 0 rejected/no complete, 1 first RX commit, 2 TX final completion.
 * Output response is queued for a 15ms radio turnaround, never busy-waited. */
/* 执行当前语句。 */
uint8_t FastPair_Input(uint32_t now,const LinkMessage *m);
/* 根据编译条件选择对应实现。 */
#endif
