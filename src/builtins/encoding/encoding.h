#ifndef TOLLVM_ENCODING_H
#define TOLLVM_ENCODING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_encoding_str_result;

/* Base64 (string ↔ string) */
tollvm_encoding_str_result tollvm_base64EncodeStr(const char* s, size_t s_len);
tollvm_encoding_str_result tollvm_base64DecodeStr(const char* s, size_t s_len);

/* URL encoding */
tollvm_encoding_str_result tollvm_urlEncode(const char* s, size_t s_len);
tollvm_encoding_str_result tollvm_urlDecode(const char* s, size_t s_len);

/* Base64 with Vec<uint8> */
typedef struct { void* ptr; size_t len; size_t cap; } tollvm_enc_vec_header;

tollvm_encoding_str_result tollvm_base64EncodeVec(const tollvm_enc_vec_header* data);
void tollvm_base64DecodeVec(tollvm_enc_vec_header* out, const char* s, size_t s_len);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_ENCODING_H */
