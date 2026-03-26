#ifndef LUX_PATH_H
#define LUX_PATH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_path_str_result;

/* Join */
lux_path_str_result lux_pathJoin(const char* a, size_t a_len,
                                       const char* b, size_t b_len);

/* Components */
lux_path_str_result lux_parent(const char* p, size_t len);
lux_path_str_result lux_fileName(const char* p, size_t len);
lux_path_str_result lux_stem(const char* p, size_t len);
lux_path_str_result lux_extension(const char* p, size_t len);

/* Queries */
int32_t lux_isAbsolute(const char* p, size_t len);
int32_t lux_isRelative(const char* p, size_t len);

/* Transform */
lux_path_str_result lux_normalize(const char* p, size_t len);
lux_path_str_result lux_toAbsolute(const char* p, size_t len);

/* Separator */
uint8_t lux_separator(void);

/* Modify */
lux_path_str_result lux_withExtension(const char* p, size_t p_len,
                                             const char* ext, size_t ext_len);
lux_path_str_result lux_withFileName(const char* p, size_t p_len,
                                            const char* name, size_t name_len);

/* Vec-based path operations */
typedef struct { void* ptr; size_t len; size_t cap; } lux_path_vec_header;

lux_path_str_result lux_joinAllVec(const lux_path_vec_header* parts);

#ifdef __cplusplus
}
#endif

#endif /* LUX_PATH_H */
