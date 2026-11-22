/* 文件说明：点对点链路加密接口，基于 Curve25519 和 XSalsa20-Poly1305。
 * 注释范围：自编应用代码；第三方 TweetNaCl 保持原始实现。
 */
#ifndef LINK_CRYPTO_H
#define LINK_CRYPTO_H
#include <stdint.h>
#define LINK_PUBLIC_KEY_BYTES 32U
#define LINK_SECRET_KEY_BYTES 32U
#define LINK_KEY_BYTES 32U
#define LINK_CHALLENGE_BYTES 8U
#define LINK_PROOF_BYTES 8U
#define LINK_CIPHER_BYTES 40U
#define LINK_PLAIN_BYTES 24U
/** @brief 从安装密钥、设备 ID、持久化 epoch 和角色派生伪随机流。 */
void LinkCrypto_Init(uint32_t device_id,uint32_t persistent_epoch,uint32_t role);
void LinkCrypto_Mac(uint8_t out[64],const uint8_t *input,uint16_t length,
                    const uint8_t key[32]);
/** @brief 生成一次配对使用的 Curve25519 临时密钥对。 */
void LinkCrypto_Keypair(uint8_t public_key[LINK_PUBLIC_KEY_BYTES],
                        uint8_t secret_key[LINK_SECRET_KEY_BYTES]);
/** @brief 由对方公钥和本机私钥计算共享链路密钥。 */
uint8_t LinkCrypto_Shared(uint8_t key[LINK_KEY_BYTES],
                       const uint8_t peer_public[LINK_PUBLIC_KEY_BYTES],
                       const uint8_t own_secret[LINK_SECRET_KEY_BYTES]);
/** @brief 生成配对双方共同可验证的 64 位确认值。 */
void LinkCrypto_Proof(uint8_t proof[LINK_PROOF_BYTES],
                      const uint8_t key[LINK_KEY_BYTES],
                      uint32_t tx_id,uint32_t rx_id,
                      uint32_t session,uint32_t token,
                      const uint8_t tx_public[LINK_PUBLIC_KEY_BYTES],
                      const uint8_t rx_public[LINK_PUBLIC_KEY_BYTES],
                      const uint8_t challenge[LINK_CHALLENGE_BYTES]);
/** @brief 对 24 字节应用数据做认证加密，输出 40 字节密文和认证码。 */
void LinkCrypto_Seal(uint8_t cipher[LINK_CIPHER_BYTES],
                     const uint8_t plain[LINK_PLAIN_BYTES],
                     const uint8_t nonce[24],
                     const uint8_t key[LINK_KEY_BYTES]);
/** @brief 验证并解密数据；认证失败返回 0。 */
uint8_t LinkCrypto_Open(uint8_t plain[LINK_PLAIN_BYTES],
                        const uint8_t cipher[LINK_CIPHER_BYTES],
                        const uint8_t nonce[24],
                        const uint8_t key[LINK_KEY_BYTES]);
#endif
