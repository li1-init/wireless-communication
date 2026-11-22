/* 文件说明：链路加密与随机数适配；密钥交换使用 TweetNaCl Curve25519。
 * 注释范围：自编应用代码；第三方 TweetNaCl 保持原始实现。
 */
#include "link_crypto.h"
#include "tweetnacl.h"
#include "provision_secret.h"
static uint8_t pool[64];
static uint8_t pool_pos=64U;
static uint32_t pool_counter;
static uint32_t seed_device,seed_time,seed_clock;
/** @brief 小端写入 32 位值。 */
static void Put32(uint8_t *p,uint32_t value)
{
    uint8_t i;
    for(i=0;i<4U;++i) p[i]=(uint8_t)(value>>(i*8U));
}
/* HMAC-SHA512. Input is bounded by the longest pairing transcript. */
void LinkCrypto_Mac(uint8_t out[64],const uint8_t *input,uint16_t length,
                    const uint8_t key[32])
{
    uint8_t buf[256],inner[64];
    uint16_t i;
    for(i=0;i<128U;++i) buf[i]=(uint8_t)((i<32U?key[i]:0U)^0x36U);
    for(i=0;i<length;++i) buf[128U+i]=input[i];
    crypto_hash(inner,buf,128U+length);
    for(i=0;i<128U;++i) buf[i]=(uint8_t)((i<32U?key[i]:0U)^0x5cU);
    for(i=0;i<64U;++i) buf[128U+i]=inner[i];
    crypto_hash(out,buf,192U);
}
/** @brief Seed from a provisioned secret and a persistent unique epoch. */
void LinkCrypto_Init(uint32_t device_id,uint32_t timing,uint32_t clock_hz)
{
    uint8_t input[32];
    seed_device=device_id;
    seed_time=timing;
    seed_clock=clock_hz;
    Put32(input+0,device_id);
    Put32(input+4,timing);
    Put32(input+8,clock_hz);
    Put32(input+12,0x524e4734UL);
    Put32(input+16,pool_counter++);
    Put32(input+20,device_id^timing^clock_hz);
    Put32(input+24,~device_id);
    Put32(input+28,0x50524634UL);
    LinkCrypto_Mac(pool,input,sizeof(input),provision_secret);
    pool_pos=0U;
}
/** @brief TweetNaCl 随机源；每次取完 64 字节重新混合时钟和计数器。 */
void randombytes(unsigned char *out,unsigned long long length)
{
    unsigned long long i;
    for(i=0;i<length;++i) {
        if(pool_pos>=sizeof(pool)) {
            uint8_t input[32];
            Put32(input+0,seed_device^pool[0]);
            Put32(input+4,seed_time^pool[7]);
            Put32(input+8,seed_clock^pool[15]);
            Put32(input+12,pool_counter++);
            Put32(input+16,seed_time);
            Put32(input+20,(uint32_t)length);
            Put32(input+24,pool[23]);
            Put32(input+28,pool[31]);
            LinkCrypto_Mac(pool,input,sizeof(input),provision_secret);
            pool_pos=0U;
        }
        out[i]=pool[pool_pos++];
    }
}
/** @brief 生成 Curve25519 临时密钥对。 */
void LinkCrypto_Keypair(uint8_t public_key[LINK_PUBLIC_KEY_BYTES],
                        uint8_t secret_key[LINK_SECRET_KEY_BYTES])
{
    (void)crypto_box_keypair(public_key,secret_key);
}
/** @brief 计算双方共享的 precomputed key。 */
uint8_t LinkCrypto_Shared(uint8_t key[LINK_KEY_BYTES],
                       const uint8_t peer_public[LINK_PUBLIC_KEY_BYTES],
                       const uint8_t own_secret[LINK_SECRET_KEY_BYTES])
{
    uint8_t raw[32],i,nonzero=0;
    static const uint8_t zero[16]={0};
    static const uint8_t sigma[16]="expand 32-byte k";
    crypto_scalarmult(raw,own_secret,peer_public);
    for(i=0;i<32;++i) nonzero|=raw[i];
    if(!nonzero) return 0;
    /* Same derivation as TweetNaCl crypto_box_beforenm, reusing the
     * scalar result already computed for low-order-point rejection. */
    if(crypto_core_hsalsa20(key,zero,raw,sigma)!=0) return 0;
    return 1;
}
/** @brief 用共享密钥计算配对双方可验证的 HMAC 前缀。 */
void LinkCrypto_Proof(uint8_t proof[LINK_PROOF_BYTES],
                      const uint8_t key[LINK_KEY_BYTES],
                      uint32_t tx_id,uint32_t rx_id,
                      uint32_t session,uint32_t token,
                      const uint8_t tx_public[LINK_PUBLIC_KEY_BYTES],
                      const uint8_t rx_public[LINK_PUBLIC_KEY_BYTES],
                      const uint8_t challenge[LINK_CHALLENGE_BYTES])
{
    uint8_t input[96],full[64],authenticated[64];
    input[0]='T'; input[1]='R'; input[2]='P'; input[3]='A';
    input[4]='I'; input[5]='R'; input[6]='0'; input[7]='4';
    Put32(input+8,tx_id);
    Put32(input+12,rx_id);
    Put32(input+16,session);
    Put32(input+20,token);
    {
        uint8_t i;
        for(i=0;i<32U;++i) {
            input[24U+i]=tx_public[i];
            input[56U+i]=rx_public[i];
        }
        for(i=0;i<8U;++i) input[88U+i]=challenge[i];
    }
    /* Mix installation authentication with the ephemeral DH shared secret.
     * A radio-only outsider cannot substitute public keys during enrollment. */
    LinkCrypto_Mac(authenticated,input,sizeof(input),provision_secret);
    LinkCrypto_Mac(full,authenticated,sizeof(authenticated),key);
    {
        uint8_t i;
        for(i=0;i<LINK_PROOF_BYTES;++i) proof[i]=full[i];
    }
}
/** @brief 对应用数据做 XSalsa20-Poly1305 认证加密。 */
void LinkCrypto_Seal(uint8_t cipher[LINK_CIPHER_BYTES],
                     const uint8_t plain[LINK_PLAIN_BYTES],
                     const uint8_t nonce[24],
                     const uint8_t key[LINK_KEY_BYTES])
{
    uint8_t input[32U+LINK_PLAIN_BYTES];
    uint8_t output[32U+LINK_PLAIN_BYTES];
    uint8_t i;
    for(i=0;i<32U;++i) input[i]=0U;
    for(i=0;i<LINK_PLAIN_BYTES;++i) input[32U+i]=plain[i];
    if(crypto_secretbox(output,input,sizeof(input),nonce,key)!=0) {
        for(i=0;i<LINK_CIPHER_BYTES;++i) cipher[i]=0U;
        return;
    }
    for(i=0;i<LINK_CIPHER_BYTES;++i) cipher[i]=output[16U+i];
}
/** @brief 验证认证码并恢复应用数据。 */
uint8_t LinkCrypto_Open(uint8_t plain[LINK_PLAIN_BYTES],
                        const uint8_t cipher[LINK_CIPHER_BYTES],
                        const uint8_t nonce[24],
                        const uint8_t key[LINK_KEY_BYTES])
{
    uint8_t input[16U+LINK_CIPHER_BYTES];
    uint8_t output[32U+LINK_PLAIN_BYTES];
    uint8_t i;
    for(i=0;i<16U;++i) input[i]=0U;
    for(i=0;i<LINK_CIPHER_BYTES;++i) input[16U+i]=cipher[i];
    if(crypto_secretbox_open(output,input,sizeof(output),nonce,key)!=0) return 0U;
    for(i=0;i<32U;++i) if(output[i]) return 0U;
    for(i=0;i<LINK_PLAIN_BYTES;++i) plain[i]=output[32U+i];
    return 1U;
}
