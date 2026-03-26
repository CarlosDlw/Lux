#ifndef TOLLVM_COMPRESS_H
#define TOLLVM_COMPRESS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_compress_str_result;

tollvm_compress_str_result tollvm_gzipCompress(const char* s, size_t slen);
tollvm_compress_str_result tollvm_gzipDecompress(const char* s, size_t slen);
tollvm_compress_str_result tollvm_deflate(const char* s, size_t slen);
tollvm_compress_str_result tollvm_inflate(const char* s, size_t slen);
tollvm_compress_str_result tollvm_compressLevel(const char* s, size_t slen, int32_t level);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_COMPRESS_H */
