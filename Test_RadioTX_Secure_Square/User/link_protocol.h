/* 文件说明：安全点对点协议：20 字节头、60 字节定长包和认证加密数据。
 * 注释范围：自编应用代码；第三方 TweetNaCl 保持原始实现。
 */
#ifndef LINK_PROTOCOL_H
#define LINK_PROTOCOL_H
#include <stdint.h>
#include "link_crypto.h"
#define LINK_PACKET_SIZE 60U
#define LINK_HEADER_SIZE 20U
#define LINK_VERSION 4U
#define LINK_CHANNEL 0U
#define LINK_ACK_TIMEOUT_MS 500UL
#define LINK_PAIR_INTERVAL_MS 600UL
#define LINK_PAIR_TIMEOUT_MS 30000UL
enum { LINK_DATA=0x10, LINK_DATA_ACK=0x11, LINK_PAIR_COMMIT=0x22, LINK_PAIR_DONE=0x23,
       LINK_PAIR_REQUEST=0x20, LINK_PAIR_ACK=0x21 };
enum { LINK_BEACON=0, LINK_SWITCH=1, LINK_USER=2 };
#define LINK_PAIR_ACK_OK 1U
#define LINK_INTERVAL_MS 1000UL
typedef struct {
    uint32_t session;
    uint32_t sender_id;
    uint32_t receiver_id;
    uint32_t token;
    uint8_t type;
    uint8_t payload;
    uint8_t proof[8]; /* Enrollment commit authenticates the transmitter too. */
} LinkMessage;
typedef struct {
    LinkMessage message;
    uint8_t public_key[LINK_PUBLIC_KEY_BYTES];
    uint8_t extra[8];
} LinkPair;
/** @brief 仅解析未认证的公共头，用于按 sender_id 查找链路密钥。 */
uint8_t Link_ParseHeader(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *message);
/** @brief 用指定链路密钥认证加密普通数据或 ACK。 */
void Link_EncodeData(uint8_t packet[LINK_PACKET_SIZE],
                     const LinkMessage *message,
                     const uint8_t key[LINK_KEY_BYTES]);
/** @brief 用指定链路密钥验证并解密数据；认证失败返回 0。 */
uint8_t Link_DecodeData(const uint8_t packet[LINK_PACKET_SIZE],
                        LinkMessage *message,
                        const uint8_t key[LINK_KEY_BYTES]);
/** @brief 编码对码请求，携带发送端临时公钥和挑战值。 */
void Link_EncodePairRequest(uint8_t packet[LINK_PACKET_SIZE],
                            const LinkMessage *message,
                            const uint8_t public_key[LINK_PUBLIC_KEY_BYTES],
                            const uint8_t challenge[LINK_CHALLENGE_BYTES]);
/** @brief 检查并解析对码请求。 */
uint8_t Link_DecodePairRequest(const uint8_t packet[LINK_PACKET_SIZE],LinkPair *pair);
/** @brief 编码对码确认，携带接收端临时公钥和共享密钥证明。 */
void Link_EncodePairAck(uint8_t packet[LINK_PACKET_SIZE],
                        const LinkMessage *message,
                        const uint8_t public_key[LINK_PUBLIC_KEY_BYTES],
                        const uint8_t proof[LINK_PROOF_BYTES]);
/** @brief 检查并解析对码确认。 */
uint8_t Link_DecodePairAck(const uint8_t packet[LINK_PACKET_SIZE],LinkPair *pair);
/** @brief 计算 CRC-16/CCITT-FALSE，保留给存储和非加密诊断使用。 */
uint16_t Link_Crc16(const uint8_t *data,uint8_t length);
#endif
