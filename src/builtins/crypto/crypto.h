#ifndef TOLLVM_CRYPTO_H
#define TOLLVM_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_crypto_str_result;

tollvm_crypto_str_result tollvm_md5String(const char* s, size_t slen);
tollvm_crypto_str_result tollvm_sha1String(const char* s, size_t slen);
tollvm_crypto_str_result tollvm_sha256String(const char* s, size_t slen);
tollvm_crypto_str_result tollvm_sha512String(const char* s, size_t slen);

/* Vec<uint8> variants */
typedef struct { void* ptr; size_t len; size_t cap; } tollvm_crypto_vec_header;

tollvm_crypto_str_result tollvm_md5Bytes(const tollvm_crypto_vec_header* data);
tollvm_crypto_str_result tollvm_sha1Bytes(const tollvm_crypto_vec_header* data);
tollvm_crypto_str_result tollvm_sha256Bytes(const tollvm_crypto_vec_header* data);
tollvm_crypto_str_result tollvm_sha512Bytes(const tollvm_crypto_vec_header* data);
void tollvm_hmacSha256(tollvm_crypto_vec_header* out,
                       const tollvm_crypto_vec_header* key,
                       const tollvm_crypto_vec_header* data);
void tollvm_randomBytes(tollvm_crypto_vec_header* out, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_CRYPTO_H */
