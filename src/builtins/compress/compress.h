#ifndef LUX_COMPRESS_H
#define LUX_COMPRESS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_compress_str_result;

lux_compress_str_result lux_gzipCompress(const char* s, size_t slen);
lux_compress_str_result lux_gzipDecompress(const char* s, size_t slen);
lux_compress_str_result lux_deflate(const char* s, size_t slen);
lux_compress_str_result lux_inflate(const char* s, size_t slen);
lux_compress_str_result lux_compressLevel(const char* s, size_t slen, int32_t level);

#ifdef __cplusplus
}
#endif

#endif /* LUX_COMPRESS_H */
