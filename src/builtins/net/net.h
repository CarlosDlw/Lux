#ifndef TOLLVM_NET_H
#define TOLLVM_NET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_net_str_result;

/* TCP */
int32_t  tollvm_tcpConnect(const char* host, size_t hostLen, uint16_t port);
int32_t  tollvm_tcpListen(const char* host, size_t hostLen, uint16_t port);
int32_t  tollvm_tcpAccept(int32_t fd);
int64_t  tollvm_tcpSend(int32_t fd, const char* data, size_t dataLen);
tollvm_net_str_result tollvm_tcpRecv(int32_t fd, size_t maxLen);

/* UDP */
int32_t  tollvm_udpBind(const char* host, size_t hostLen, uint16_t port);
int64_t  tollvm_udpSendTo(int32_t fd, const char* data, size_t dataLen,
                           const char* host, size_t hostLen, uint16_t port);
tollvm_net_str_result tollvm_udpRecvFrom(int32_t fd, size_t maxLen);

/* General */
void     tollvm_netClose(int32_t fd);
void     tollvm_netSetTimeout(int32_t fd, uint64_t ms);
tollvm_net_str_result tollvm_netResolve(const char* host, size_t hostLen);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_NET_H */
