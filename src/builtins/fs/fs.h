#ifndef TOLLVM_FS_H
#define TOLLVM_FS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_fs_str_result;

/* Read / Write */
tollvm_fs_str_result tollvm_readFile(const char* path, size_t path_len);
void tollvm_writeFile(const char* path, size_t path_len,
                      const char* data, size_t data_len);
void tollvm_appendFile(const char* path, size_t path_len,
                       const char* data, size_t data_len);

/* Queries */
int32_t tollvm_exists(const char* path, size_t path_len);
int32_t tollvm_isFile(const char* path, size_t path_len);
int32_t tollvm_isDir(const char* path, size_t path_len);
int64_t tollvm_fileSize(const char* path, size_t path_len);

/* Mutation */
int32_t tollvm_remove(const char* path, size_t path_len);
int32_t tollvm_removeDir(const char* path, size_t path_len);
int32_t tollvm_fsRename(const char* from, size_t from_len,
                        const char* to,   size_t to_len);
int32_t tollvm_mkdir(const char* path, size_t path_len);
int32_t tollvm_mkdirAll(const char* path, size_t path_len);

/* Working directory */
tollvm_fs_str_result tollvm_cwd(void);
int32_t tollvm_setCwd(const char* path, size_t path_len);

/* Temp */
tollvm_fs_str_result tollvm_tempDir(void);

/* Vec-returning functions */
typedef struct { void* ptr; size_t len; size_t cap; } tollvm_fs_vec_header;

void tollvm_listDir(tollvm_fs_vec_header* out, const char* path, size_t path_len);
void tollvm_readFileBytes(tollvm_fs_vec_header* out, const char* path, size_t path_len);
void tollvm_writeFileBytes(const char* path, size_t path_len,
                           const tollvm_fs_vec_header* data);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_FS_H */
