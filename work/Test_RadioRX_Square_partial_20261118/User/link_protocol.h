/* 文件说明：协议模块：定义数据与确认帧，负责编码、解码和 CRC16 校验。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
#ifndef LINK_PROTOCOL_H
#define LINK_PROTOCOL_H
#include <stdint.h>
#define LINK_PACKET_SIZE 16U
#define LINK_INTERVAL_MS 1000UL
#define LINK_CHANNEL 0U
#define LINK_VERSION 3U
#define LINK_EXPECTED_SENDER_ID 3155002632UL
enum { LINK_DATA=0x10, LINK_DATA_ACK=0x11, LINK_PAIR_REQUEST=0x20, LINK_PAIR_ACK=0x21 };
enum { LINK_BEACON=0, LINK_SWITCH=1, LINK_USER=2 };
#define LINK_PAIR_ACK_OK 1U
typedef struct {
    uint32_t session;
    uint32_t sender_id;
    uint8_t type;
    uint8_t payload;
    uint8_t token;
} LinkMessage;
/** @brief 组装公共帧头、类型、数据字段及末尾 CRC16。 */
void Link_Encode(uint8_t packet[LINK_PACKET_SIZE],const LinkMessage *message);
/** @brief 检查帧头、版本、类型、字段和 CRC，合法后才输出消息结构。 */
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *message);
/** @brief 计算 CRC-16/CCITT-FALSE：初值 FFFF，多项式 1021。 */
uint16_t Link_Crc16(const uint8_t *data,uint8_t length);
/** @brief 旧版序号比较辅助函数；当前普通数据接受逻辑已不使用它。 */
uint8_t Link_IsNewer(uint32_t session,uint8_t sequence,
                     uint32_t last_session,uint8_t last_sequence,uint8_t seen);
#endif
