#ifndef LUX_NET_H
#define LUX_NET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_net_str_result;

/* TCP */
int32_t  lux_tcpConnect(const char* host, size_t hostLen, uint16_t port);
int32_t  lux_tcpListen(const char* host, size_t hostLen, uint16_t port);
int32_t  lux_tcpAccept(int32_t fd);
int64_t  lux_tcpSend(int32_t fd, const char* data, size_t dataLen);
lux_net_str_result lux_tcpRecv(int32_t fd, size_t maxLen);

/* UDP */
int32_t  lux_udpBind(const char* host, size_t hostLen, uint16_t port);
int64_t  lux_udpSendTo(int32_t fd, const char* data, size_t dataLen,
                           const char* host, size_t hostLen, uint16_t port);
lux_net_str_result lux_udpRecvFrom(int32_t fd, size_t maxLen);

/* General */
void     lux_netClose(int32_t fd);
void     lux_netSetTimeout(int32_t fd, uint64_t ms);
lux_net_str_result lux_netResolve(const char* host, size_t hostLen);

#ifdef __cplusplus
}
#endif

#endif /* LUX_NET_H */
