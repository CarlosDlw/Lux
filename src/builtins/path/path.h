#ifndef TOLLVM_PATH_H
#define TOLLVM_PATH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_path_str_result;

/* Join */
tollvm_path_str_result tollvm_pathJoin(const char* a, size_t a_len,
                                       const char* b, size_t b_len);

/* Components */
tollvm_path_str_result tollvm_parent(const char* p, size_t len);
tollvm_path_str_result tollvm_fileName(const char* p, size_t len);
tollvm_path_str_result tollvm_stem(const char* p, size_t len);
tollvm_path_str_result tollvm_extension(const char* p, size_t len);

/* Queries */
int32_t tollvm_isAbsolute(const char* p, size_t len);
int32_t tollvm_isRelative(const char* p, size_t len);

/* Transform */
tollvm_path_str_result tollvm_normalize(const char* p, size_t len);
tollvm_path_str_result tollvm_toAbsolute(const char* p, size_t len);

/* Separator */
uint8_t tollvm_separator(void);

/* Modify */
tollvm_path_str_result tollvm_withExtension(const char* p, size_t p_len,
                                             const char* ext, size_t ext_len);
tollvm_path_str_result tollvm_withFileName(const char* p, size_t p_len,
                                            const char* name, size_t name_len);

/* Vec-based path operations */
typedef struct { void* ptr; size_t len; size_t cap; } tollvm_path_vec_header;

tollvm_path_str_result tollvm_joinAllVec(const tollvm_path_vec_header* parts);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_PATH_H */
