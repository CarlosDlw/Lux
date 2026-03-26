#define _POSIX_C_SOURCE 200809L

#include "process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/utsname.h>

/* ── helper: null-terminate a (ptr, len) pair ────────────────────────── */
static char* make_cstr(const char* s, size_t len) {
    char* buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, s, len);
    buf[len] = '\0';
    return buf;
}

static lux_proc_str_result make_result(const char* s) {
    lux_proc_str_result res;
    if (s) {
        size_t len = strlen(s);
        char* out = (char*)malloc(len);
        if (out) {
            memcpy(out, s, len);
            res.ptr = out;
            res.len = len;
        } else {
            res.ptr = NULL;
            res.len = 0;
        }
    } else {
        res.ptr = NULL;
        res.len = 0;
    }
    return res;
}

/* ── abort ───────────────────────────────────────────────────────────── */
void lux_abort(void) {
    abort();
}

/* ── env ─────────────────────────────────────────────────────────────── */
lux_proc_str_result lux_env(const char* name, size_t name_len) {
    char* cname = make_cstr(name, name_len);
    if (!cname) {
        lux_proc_str_result r = {NULL, 0};
        return r;
    }
    const char* val = getenv(cname);
    free(cname);
    return make_result(val);
}

/* ── setEnv ──────────────────────────────────────────────────────────── */
void lux_setEnv(const char* name, size_t name_len,
                   const char* value, size_t value_len) {
    char* cname  = make_cstr(name,  name_len);
    char* cvalue = make_cstr(value, value_len);
    if (cname && cvalue)
        setenv(cname, cvalue, 1);
    free(cname);
    free(cvalue);
}

/* ── hasEnv ──────────────────────────────────────────────────────────── */
int32_t lux_hasEnv(const char* name, size_t name_len) {
    char* cname = make_cstr(name, name_len);
    if (!cname) return 0;
    const char* val = getenv(cname);
    free(cname);
    return val != NULL ? 1 : 0;
}

/* ── exec ────────────────────────────────────────────────────────────── */
int32_t lux_exec(const char* cmd, size_t cmd_len) {
    char* ccmd = make_cstr(cmd, cmd_len);
    if (!ccmd) return -1;
    int status = system(ccmd);
    free(ccmd);
    if (status == -1) return -1;
    /* WEXITSTATUS extracts the exit code */
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* ── execOutput ──────────────────────────────────────────────────────── */
lux_proc_str_result lux_execOutput(const char* cmd, size_t cmd_len) {
    lux_proc_str_result res = {NULL, 0};
    char* ccmd = make_cstr(cmd, cmd_len);
    if (!ccmd) return res;

    FILE* fp = popen(ccmd, "r");
    free(ccmd);
    if (!fp) return res;

    size_t cap = 4096;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) { pclose(fp); return res; }

    size_t nread;
    while ((nread = fread(buf + len, 1, cap - len, fp)) > 0) {
        len += nread;
        if (len == cap) {
            cap *= 2;
            char* tmp = (char*)realloc(buf, cap);
            if (!tmp) { free(buf); pclose(fp); return res; }
            buf = tmp;
        }
    }
    pclose(fp);

    res.ptr = buf;
    res.len = len;
    return res;
}

/* ── pid ─────────────────────────────────────────────────────────────── */
int32_t lux_pid(void) {
    return (int32_t)getpid();
}

/* ── platform ────────────────────────────────────────────────────────── */
lux_proc_str_result lux_platform(void) {
#if defined(__linux__)
    return make_result("linux");
#elif defined(__APPLE__)
    return make_result("macos");
#elif defined(_WIN32)
    return make_result("windows");
#else
    return make_result("unknown");
#endif
}

/* ── arch ────────────────────────────────────────────────────────────── */
lux_proc_str_result lux_arch(void) {
#if defined(__x86_64__) || defined(_M_X64)
    return make_result("x86_64");
#elif defined(__aarch64__) || defined(_M_ARM64)
    return make_result("aarch64");
#elif defined(__i386__) || defined(_M_IX86)
    return make_result("x86");
#elif defined(__arm__)
    return make_result("arm");
#elif defined(__riscv)
    return make_result("riscv");
#else
    return make_result("unknown");
#endif
}

/* ── homeDir ─────────────────────────────────────────────────────────── */
lux_proc_str_result lux_homeDir(void) {
    const char* home = getenv("HOME");
    return make_result(home);
}

/* ── executablePath ──────────────────────────────────────────────────── */
lux_proc_str_result lux_executablePath(void) {
    lux_proc_str_result res = {NULL, 0};
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        char* out = (char*)malloc((size_t)len);
        if (out) {
            memcpy(out, buf, (size_t)len);
            res.ptr = out;
            res.len = (size_t)len;
        }
    }
    return res;
}
