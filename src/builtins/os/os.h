#ifndef LUX_OS_H
#define LUX_OS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_os_str_result;

/* Process info */
int32_t  lux_osGetpid(void);
int32_t  lux_osGetppid(void);
uint32_t lux_osGetuid(void);
uint32_t lux_osGetgid(void);

/* System info */
lux_os_str_result lux_osHostname(void);
size_t   lux_osPageSize(void);

/* Error handling */
int32_t  lux_osErrno(void);
lux_os_str_result lux_osStrerror(int32_t code);

/* Signals */
int32_t  lux_osKill(int32_t pid, int32_t sig);

/* File descriptors */
int32_t  lux_osDup(int32_t fd);
int32_t  lux_osDup2(int32_t oldfd, int32_t newfd);
int32_t  lux_osCloseFd(int32_t fd);

#ifdef __cplusplus
}
#endif

#endif /* LUX_OS_H */
