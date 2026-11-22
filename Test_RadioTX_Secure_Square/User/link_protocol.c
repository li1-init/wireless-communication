/* 文件说明：安全点对点协议编解码与认证加密。
 * 注释范围：自编应用代码；第三方 TweetNaCl 保持原始实现。
 */
#include <string.h>
#include "link_protocol.h"
/** @brief 计算 CRC-16/CCITT-FALSE。 */
uint16_t Link_Crc16(const uint8_t *data,uint8_t length)
{
    uint16_t crc=0xFFFFU;
    uint8_t i,bit;
    for(i=0;i<length;++i) {
        crc^=(uint16_t)data[i]<<8;
        for(bit=0;bit<8U;++bit)
            crc=(uint16_t)((crc&0x8000U)?((uint32_t)crc<<1U)^0x1021U:(uint32_t)crc<<1U);
    }
    return crc;
}
/** @brief 小端写入 32 位值。 */
static void Put32(uint8_t *p,uint32_t value)
{
    uint8_t i;
    for(i=0;i<4U;++i) p[i]=(uint8_t)(value>>(i*8U));
}
/** @brief 从四个小端字节读取 32 位值。 */
static uint32_t Get32(const uint8_t *p)
{
    return (uint32_t)p[0]|((uint32_t)p[1]<<8U)|
           ((uint32_t)p[2]<<16U)|((uint32_t)p[3]<<24U);
}
/** @brief 写入公共 20 字节头。 */
static void Header(uint8_t packet[LINK_PACKET_SIZE],const LinkMessage *message)
{
    packet[0]=0x54U; packet[1]=0x52U; packet[2]=LINK_VERSION; packet[3]=message->type;
    Put32(packet+4,message->sender_id);
    Put32(packet+8,message->receiver_id);
    Put32(packet+12,message->session);
    Put32(packet+16,message->token);
}
/** @brief 从公共头恢复路由字段；此结果在密钥验证前不能当作可信数据。 */
uint8_t Link_ParseHeader(const uint8_t packet[LINK_PACKET_SIZE],LinkMessage *message)
{
    if(packet[0]!=0x54U || packet[1]!=0x52U || packet[2]!=LINK_VERSION) return 0U;
    message->type=packet[3];
    message->sender_id=Get32(packet+4);
    message->receiver_id=Get32(packet+8);
    message->session=Get32(packet+12);
    message->token=Get32(packet+16);
    message->payload=0U;
    return 1U;
}
/** @brief 由认证头派生 24 字节 nonce；包头改动会使认证失败。 */
static void Nonce(uint8_t nonce[24],const uint8_t packet[LINK_PACKET_SIZE])
{
    uint8_t i;
    for(i=0;i<20U;++i) nonce[i]=packet[i];
    nonce[20]='N'; nonce[21]='O'; nonce[22]='N'; nonce[23]='4';
}
/** @brief 用链路密钥加密数据或 ACK。 */
void Link_EncodeData(uint8_t packet[LINK_PACKET_SIZE],
                     const LinkMessage *message,
                     const uint8_t key[LINK_KEY_BYTES])
{
    uint8_t plain[LINK_PLAIN_BYTES],nonce[24];
    Header(packet,message);
    Put32(plain+0,message->sender_id);
    Put32(plain+4,message->receiver_id);
    Put32(plain+8,message->session);
    Put32(plain+12,message->token);
    plain[16]=message->payload;
    plain[17]=message->type;
    plain[18]='S'; plain[19]='E'; plain[20]='C';
    plain[21]='0'; plain[22]='0'; plain[23]='4';
    if(message->type==LINK_PAIR_COMMIT) memcpy(plain+16,message->proof,8);
    Nonce(nonce,packet);
    LinkCrypto_Seal(packet+LINK_HEADER_SIZE,plain,nonce,key);
    {
        uint8_t i,nonzero=0;
        for(i=0;i<LINK_CIPHER_BYTES;++i) nonzero|=packet[LINK_HEADER_SIZE+i];
        if(!nonzero) memset(packet,0,LINK_PACKET_SIZE);
    }
}
/** @brief 验证并解密数据；同时核对明文内复制的全部身份字段。 */
uint8_t Link_DecodeData(const uint8_t packet[LINK_PACKET_SIZE],
                        LinkMessage *message,
                        const uint8_t key[LINK_KEY_BYTES])
{
    uint8_t plain[LINK_PLAIN_BYTES],nonce[24];
    if(!Link_ParseHeader(packet,message)) return 0U;
    if(message->type!=LINK_DATA && message->type!=LINK_DATA_ACK &&
       message->type!=LINK_PAIR_COMMIT && message->type!=LINK_PAIR_DONE) return 0U;
    if(!message->sender_id || !message->receiver_id || !message->session) return 0U;
    Nonce(nonce,packet);
    if(!LinkCrypto_Open(plain,packet+LINK_HEADER_SIZE,nonce,key)) return 0U;
    if(Get32(plain+0)!=message->sender_id ||
       Get32(plain+4)!=message->receiver_id ||
       Get32(plain+8)!=message->session ||
       Get32(plain+12)!=message->token) return 0;
    if(message->type==LINK_PAIR_COMMIT) {
        memcpy(message->proof,plain+16,8);
        message->payload=LINK_PAIR_ACK_OK;
        return 1;
    }
    if(plain[17]!=message->type ||
       plain[18]!='S' || plain[19]!='E' || plain[20]!='C' ||
       plain[21]!='0' || plain[22]!='0' || plain[23]!='4') return 0U;
    message->payload=plain[16];
    if((message->type==LINK_DATA && (message->payload>>3U)<=LINK_USER) ||
       ((message->type==LINK_DATA_ACK || message->type==LINK_PAIR_DONE) &&
        message->payload==LINK_PAIR_ACK_OK)) return 1U;
    return 0U;
}
/** @brief 编码对码请求。 */
void Link_EncodePairRequest(uint8_t packet[LINK_PACKET_SIZE],
                            const LinkMessage *message,
                            const uint8_t public_key[LINK_PUBLIC_KEY_BYTES],
                            const uint8_t challenge[LINK_CHALLENGE_BYTES])
{
    uint8_t i;
    Header(packet,message);
    for(i=0;i<LINK_PUBLIC_KEY_BYTES;++i) packet[LINK_HEADER_SIZE+i]=public_key[i];
    for(i=0;i<LINK_CHALLENGE_BYTES;++i) packet[LINK_HEADER_SIZE+LINK_PUBLIC_KEY_BYTES+i]=challenge[i];
}
/** @brief 解码对码请求。 */
uint8_t Link_DecodePairRequest(const uint8_t packet[LINK_PACKET_SIZE],LinkPair *pair)
{
    uint8_t i;
    if(!Link_ParseHeader(packet,&pair->message)) return 0U;
    if(pair->message.type!=LINK_PAIR_REQUEST || pair->message.receiver_id!=0U ||
       !pair->message.sender_id || !pair->message.session || !pair->message.token) return 0U;
    for(i=0;i<LINK_PUBLIC_KEY_BYTES;++i) pair->public_key[i]=packet[LINK_HEADER_SIZE+i];
    for(i=0;i<LINK_CHALLENGE_BYTES;++i) pair->extra[i]=packet[LINK_HEADER_SIZE+LINK_PUBLIC_KEY_BYTES+i];
    return 1U;
}
/** @brief 编码对码确认。 */
void Link_EncodePairAck(uint8_t packet[LINK_PACKET_SIZE],
                        const LinkMessage *message,
                        const uint8_t public_key[LINK_PUBLIC_KEY_BYTES],
                        const uint8_t proof[LINK_PROOF_BYTES])
{
    uint8_t i;
    Header(packet,message);
    for(i=0;i<LINK_PUBLIC_KEY_BYTES;++i) packet[LINK_HEADER_SIZE+i]=public_key[i];
    for(i=0;i<LINK_PROOF_BYTES;++i) packet[LINK_HEADER_SIZE+LINK_PUBLIC_KEY_BYTES+i]=proof[i];
}
/** @brief 解码对码确认。 */
uint8_t Link_DecodePairAck(const uint8_t packet[LINK_PACKET_SIZE],LinkPair *pair)
{
    uint8_t i;
    if(!Link_ParseHeader(packet,&pair->message)) return 0U;
    if(pair->message.type!=LINK_PAIR_ACK || !pair->message.sender_id ||
       !pair->message.receiver_id || !pair->message.session || !pair->message.token) return 0U;
    for(i=0;i<LINK_PUBLIC_KEY_BYTES;++i) pair->public_key[i]=packet[LINK_HEADER_SIZE+i];
    for(i=0;i<LINK_PROOF_BYTES;++i) pair->extra[i]=packet[LINK_HEADER_SIZE+LINK_PUBLIC_KEY_BYTES+i];
    return 1U;
}
