#ifndef TOLLVM_OS_H
#define TOLLVM_OS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_os_str_result;

/* Process info */
int32_t  tollvm_osGetpid(void);
int32_t  tollvm_osGetppid(void);
uint32_t tollvm_osGetuid(void);
uint32_t tollvm_osGetgid(void);

/* System info */
tollvm_os_str_result tollvm_osHostname(void);
size_t   tollvm_osPageSize(void);

/* Error handling */
int32_t  tollvm_osErrno(void);
tollvm_os_str_result tollvm_osStrerror(int32_t code);

/* Signals */
int32_t  tollvm_osKill(int32_t pid, int32_t sig);

/* File descriptors */
int32_t  tollvm_osDup(int32_t fd);
int32_t  tollvm_osDup2(int32_t oldfd, int32_t newfd);
int32_t  tollvm_osCloseFd(int32_t fd);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_OS_H */
