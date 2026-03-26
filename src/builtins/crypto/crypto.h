#ifndef LUX_CRYPTO_H
#define LUX_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_crypto_str_result;

lux_crypto_str_result lux_md5String(const char* s, size_t slen);
lux_crypto_str_result lux_sha1String(const char* s, size_t slen);
lux_crypto_str_result lux_sha256String(const char* s, size_t slen);
lux_crypto_str_result lux_sha512String(const char* s, size_t slen);

/* Vec<uint8> variants */
typedef struct { void* ptr; size_t len; size_t cap; } lux_crypto_vec_header;

lux_crypto_str_result lux_md5Bytes(const lux_crypto_vec_header* data);
lux_crypto_str_result lux_sha1Bytes(const lux_crypto_vec_header* data);
lux_crypto_str_result lux_sha256Bytes(const lux_crypto_vec_header* data);
lux_crypto_str_result lux_sha512Bytes(const lux_crypto_vec_header* data);
void lux_hmacSha256(lux_crypto_vec_header* out,
                       const lux_crypto_vec_header* key,
                       const lux_crypto_vec_header* data);
void lux_randomBytes(lux_crypto_vec_header* out, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* LUX_CRYPTO_H */
