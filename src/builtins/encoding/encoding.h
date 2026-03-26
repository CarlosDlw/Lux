#ifndef LUX_ENCODING_H
#define LUX_ENCODING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_encoding_str_result;

/* Base64 (string ↔ string) */
lux_encoding_str_result lux_base64EncodeStr(const char* s, size_t s_len);
lux_encoding_str_result lux_base64DecodeStr(const char* s, size_t s_len);

/* URL encoding */
lux_encoding_str_result lux_urlEncode(const char* s, size_t s_len);
lux_encoding_str_result lux_urlDecode(const char* s, size_t s_len);

/* Base64 with Vec<uint8> */
typedef struct { void* ptr; size_t len; size_t cap; } lux_enc_vec_header;

lux_encoding_str_result lux_base64EncodeVec(const lux_enc_vec_header* data);
void lux_base64DecodeVec(lux_enc_vec_header* out, const char* s, size_t s_len);

#ifdef __cplusplus
}
#endif

#endif /* LUX_ENCODING_H */
