/**
 * @file link_protocol.h
 * @brief 链路帧格式及编码接口。
 */
#ifndef LINK_PROTOCOL_H
#define LINK_PROTOCOL_H

#include <stdint.h>

#define LINK_PACKET_SIZE 28U
#define LINK_VERSION 5U
#define LINK_CHANNEL 0U

enum
{
    LINK_DATA = 0x10,
    LINK_DATA_ACK = 0x11,
    LINK_PAIR_REQUEST = 0x20,
    LINK_PAIR_ACK = 0x21,
    LINK_PAIR_COMMIT = 0x22,
    LINK_PAIR_DONE = 0x23
};

/**
 * @brief 一帧链路消息的解码后表示。
 */
typedef struct
{
    uint32_t sender_id;
    uint32_t receiver_id;
    uint32_t session;
    uint32_t token;
    uint32_t binding;
    uint8_t type;
    uint8_t payload;
} LinkMessage;

/**
 * @brief 计算链路帧的 CRC16。
 * @param data 待校验数据。
 * @param length 待校验数据长度。
 * @return CRC16 校验值。
 */
uint16_t Link_Crc16(const uint8_t *data, uint8_t length);

/**
 * @brief 将链路消息编码为固定长度帧。
 * @param packet 输出帧缓冲区。
 * @param message 要编码的链路消息。
 */
void Link_Encode(uint8_t packet[LINK_PACKET_SIZE],
                 const LinkMessage *message);

/**
 * @brief 校验并解码固定长度链路帧。
 * @param packet 输入帧缓冲区。
 * @param message 输出的链路消息。
 * @return 校验和格式均正确时返回 1，否则返回 0。
 */
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],
                    LinkMessage *message);

#endif
