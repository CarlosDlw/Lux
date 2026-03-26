#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── String return type (matches lux string ABI) ──────────────────────────
typedef struct { const char* ptr; size_t len; } lux_io_string;

// ── Stdin — Text ────────────────────────────────────────────────────────────

lux_io_string lux_readLine(void);
char             lux_readChar(void);
int64_t          lux_readInt(void);
double           lux_readFloat(void);
int              lux_readBool(void);
lux_io_string lux_readAll(void);

// prompt variants: print prompt string, then read
lux_io_string lux_prompt(const char* msg, size_t msgLen);
int64_t          lux_promptInt(const char* msg, size_t msgLen);
double           lux_promptFloat(const char* msg, size_t msgLen);
int              lux_promptBool(const char* msg, size_t msgLen);

// ── Stdin — Binary ──────────────────────────────────────────────────────────

uint8_t          lux_readByte(void);

// ── Stdin — Status ──────────────────────────────────────────────────────────

int              lux_isEOF(void);

// ── Secure Input ────────────────────────────────────────────────────────────

lux_io_string lux_readPassword(void);
lux_io_string lux_promptPassword(const char* msg, size_t msgLen);

// ── Stream Control ──────────────────────────────────────────────────────────

void             lux_flush(void);
void             lux_flushErr(void);

// ── Terminal Detection ──────────────────────────────────────────────────────

int              lux_isTTY(void);
int              lux_isStdoutTTY(void);
int              lux_isStderrTTY(void);

// ── Vec-returning functions ─────────────────────────────────────────────────
// Forward declare vec header (defined in collections/vec.h)
typedef struct { void* ptr; size_t len; size_t cap; } lux_io_vec_header;

void lux_readLines(lux_io_vec_header* out);
void lux_readNBytes(lux_io_vec_header* out, size_t n);

#ifdef __cplusplus
}
#endif
