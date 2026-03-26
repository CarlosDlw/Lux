#ifndef TOLLVM_PROCESS_H
#define TOLLVM_PROCESS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_proc_str_result;

/* Process control */
void tollvm_abort(void);

/* Environment */
tollvm_proc_str_result tollvm_env(const char* name, size_t name_len);
void tollvm_setEnv(const char* name, size_t name_len,
                   const char* value, size_t value_len);
int32_t tollvm_hasEnv(const char* name, size_t name_len);

/* Execution */
int32_t tollvm_exec(const char* cmd, size_t cmd_len);
tollvm_proc_str_result tollvm_execOutput(const char* cmd, size_t cmd_len);

/* Info */
int32_t tollvm_pid(void);
tollvm_proc_str_result tollvm_platform(void);
tollvm_proc_str_result tollvm_arch(void);
tollvm_proc_str_result tollvm_homeDir(void);
tollvm_proc_str_result tollvm_executablePath(void);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_PROCESS_H */
