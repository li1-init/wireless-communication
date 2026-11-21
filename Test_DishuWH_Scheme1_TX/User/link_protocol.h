/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 根据编译条件选择对应实现。 */
#ifndef LINK_PROTOCOL_H
/* 定义编译期常量或接口宏。 */
#define LINK_PROTOCOL_H
/* 引入本模块所需的头文件接口。 */
#include <stdint.h>
/* 定义编译期常量或接口宏。 */
#define LINK_PACKET_SIZE 28U
/* 定义编译期常量或接口宏。 */
#define LINK_VERSION 5U
/* 定义编译期常量或接口宏。 */
#define LINK_CHANNEL 0U
/* 定义本模块使用的数据类型。 */
enum {LINK_DATA=0x10,LINK_DATA_ACK=0x11,LINK_PAIR_REQUEST=0x20,
/* 更新运行状态或计算结果。 */
      LINK_PAIR_ACK=0x21,LINK_PAIR_COMMIT=0x22,LINK_PAIR_DONE=0x23};
/* 定义本模块使用的数据类型。 */
typedef struct {
/* 执行当前语句。 */
    uint32_t sender_id,receiver_id,session,token,binding;
/* 执行当前语句。 */
    uint8_t type,payload;
/* 执行当前语句。 */
} LinkMessage;
/* 执行当前语句。 */
uint16_t Link_Crc16(const uint8_t *data,uint8_t length);
/* 执行当前语句。 */
void Link_Encode(uint8_t packet[LINK_PACKET_SIZE],const LinkMessage *m);
/* 执行当前语句。 */
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *m);
/* 根据编译条件选择对应实现。 */
#endif
