/**
 * @file link_protocol.c
 * @brief 链路帧 CRC、编码和解码实现。
 */
#include "link_protocol.h"

/**
 * @brief 计算链路帧的 CRC16。
 * @param data 待校验数据。
 * @param length 待校验数据长度。
 * @return CRC16 校验值。
 */
uint16_t Link_Crc16(const uint8_t *data, uint8_t length)
{
    uint16_t crc;
    uint8_t index;
    uint8_t bit;

    crc = 0xffffU;

    for (index = 0U; index < length; ++index)
    {
        crc ^= (uint16_t)data[index] << 8;

        for (bit = 0U; bit < 8U; ++bit)
        {
            if (crc & 0x8000U)
            {
                crc = (uint16_t)(((uint32_t)crc << 1) ^ 0x1021U);
            }
            else
            {
                crc = (uint16_t)((uint32_t)crc << 1);
            }
        }
    }

    return crc;
}

/**
 * @brief 将 32 位整数按小端序写入缓冲区。
 * @param data 目标缓冲区。
 * @param value 待写入数值。
 */
static void Put(uint8_t *data, uint32_t value)
{
    uint8_t index;

    for (index = 0U; index < 4U; ++index)
    {
        data[index] = (uint8_t)(value >> (8U * index));
    }
}

/**
 * @brief 从缓冲区按小端序读取 32 位整数。
 * @param data 源缓冲区。
 * @return 读取到的数值。
 */
static uint32_t Get(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

/**
 * @brief 将链路消息编码为固定长度帧。
 * @param packet 输出帧缓冲区。
 * @param message 要编码的链路消息。
 */
void Link_Encode(uint8_t packet[LINK_PACKET_SIZE],
                 const LinkMessage *message)
{
    uint16_t crc;

    packet[0] = 'T';
    packet[1] = 'R';
    packet[2] = LINK_VERSION;
    packet[3] = message->type;
    Put(packet + 4U, message->sender_id);
    Put(packet + 8U, message->receiver_id);
    Put(packet + 12U, message->session);
    Put(packet + 16U, message->token);
    Put(packet + 20U, message->binding);
    packet[24] = message->payload;
    packet[25] = 0U;

    crc = Link_Crc16(packet, 26U);
    packet[26] = (uint8_t)(crc >> 8);
    packet[27] = (uint8_t)crc;
}

/**
 * @brief 校验并解码固定长度链路帧。
 * @param packet 输入帧缓冲区。
 * @param message 输出的链路消息。
 * @return 校验和格式均正确时返回 1，否则返回 0。
 */
uint8_t Link_Decode(const uint8_t packet[LINK_PACKET_SIZE],
                    LinkMessage *message)
{
    uint16_t crc;

    crc = Link_Crc16(packet, 26U);

    if (packet[0] != 'T' ||
        packet[1] != 'R' ||
        packet[2] != LINK_VERSION ||
        packet[25] ||
        packet[26] != (uint8_t)(crc >> 8) ||
        packet[27] != (uint8_t)crc)
    {
        return 0U;
    }

    message->type = packet[3];
    message->sender_id = Get(packet + 4U);
    message->receiver_id = Get(packet + 8U);
    message->session = Get(packet + 12U);
    message->token = Get(packet + 16U);
    message->binding = Get(packet + 20U);
    message->payload = packet[24];

    if (!message->sender_id ||
        message->sender_id == 0xffffffffUL ||
        !message->session ||
        !message->token)
    {
        return 0U;
    }

    if (message->type == LINK_PAIR_REQUEST)
    {
        return (uint8_t)(
            !message->receiver_id &&
            !message->binding &&
            !message->payload);
    }

    if (!message->receiver_id ||
        message->receiver_id == 0xffffffffUL ||
        !message->binding)
    {
        return 0U;
    }

    if (message->type == LINK_DATA)
    {
        uint8_t command;

        command = (uint8_t)(message->payload >> 3);
        return (uint8_t)(command >= 1U && command <= 7U);
    }

    return (uint8_t)(
        (message->type == LINK_DATA_ACK ||
         message->type == LINK_PAIR_ACK ||
         message->type == LINK_PAIR_COMMIT ||
         message->type == LINK_PAIR_DONE) &&
        message->payload == 1U);
}
