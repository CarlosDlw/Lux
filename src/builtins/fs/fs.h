#ifndef LUX_FS_H
#define LUX_FS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_fs_str_result;

/* Read / Write */
lux_fs_str_result lux_readFile(const char* path, size_t path_len);
void lux_writeFile(const char* path, size_t path_len,
                      const char* data, size_t data_len);
void lux_appendFile(const char* path, size_t path_len,
                       const char* data, size_t data_len);

/* Queries */
int32_t lux_exists(const char* path, size_t path_len);
int32_t lux_isFile(const char* path, size_t path_len);
int32_t lux_isDir(const char* path, size_t path_len);
int64_t lux_fileSize(const char* path, size_t path_len);

/* Mutation */
int32_t lux_remove(const char* path, size_t path_len);
int32_t lux_removeDir(const char* path, size_t path_len);
int32_t lux_fsRename(const char* from, size_t from_len,
                        const char* to,   size_t to_len);
int32_t lux_mkdir(const char* path, size_t path_len);
int32_t lux_mkdirAll(const char* path, size_t path_len);

/* Working directory */
lux_fs_str_result lux_cwd(void);
int32_t lux_setCwd(const char* path, size_t path_len);

/* Temp */
lux_fs_str_result lux_tempDir(void);

/* Vec-returning functions */
typedef struct { void* ptr; size_t len; size_t cap; } lux_fs_vec_header;

void lux_listDir(lux_fs_vec_header* out, const char* path, size_t path_len);
void lux_readFileBytes(lux_fs_vec_header* out, const char* path, size_t path_len);
void lux_writeFileBytes(const char* path, size_t path_len,
                           const lux_fs_vec_header* data);

#ifdef __cplusplus
}
#endif

#endif /* LUX_FS_H */
