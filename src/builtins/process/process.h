#ifndef LUX_PROCESS_H
#define LUX_PROCESS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_proc_str_result;

/* Process control */
void lux_abort(void);

/* Environment */
lux_proc_str_result lux_env(const char* name, size_t name_len);
void lux_setEnv(const char* name, size_t name_len,
                   const char* value, size_t value_len);
int32_t lux_hasEnv(const char* name, size_t name_len);

/* Execution */
int32_t lux_exec(const char* cmd, size_t cmd_len);
lux_proc_str_result lux_execOutput(const char* cmd, size_t cmd_len);

/* Info */
int32_t lux_pid(void);
lux_proc_str_result lux_platform(void);
lux_proc_str_result lux_arch(void);
lux_proc_str_result lux_homeDir(void);
lux_proc_str_result lux_executablePath(void);

#ifdef __cplusplus
}
#endif

#endif /* LUX_PROCESS_H */
