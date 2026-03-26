#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── String return type (matches tollvm string ABI) ──────────────────────────
typedef struct { const char* ptr; size_t len; } tollvm_io_string;

// ── Stdin — Text ────────────────────────────────────────────────────────────

tollvm_io_string tollvm_readLine(void);
char             tollvm_readChar(void);
int64_t          tollvm_readInt(void);
double           tollvm_readFloat(void);
int              tollvm_readBool(void);
tollvm_io_string tollvm_readAll(void);

// prompt variants: print prompt string, then read
tollvm_io_string tollvm_prompt(const char* msg, size_t msgLen);
int64_t          tollvm_promptInt(const char* msg, size_t msgLen);
double           tollvm_promptFloat(const char* msg, size_t msgLen);
int              tollvm_promptBool(const char* msg, size_t msgLen);

// ── Stdin — Binary ──────────────────────────────────────────────────────────

uint8_t          tollvm_readByte(void);

// ── Stdin — Status ──────────────────────────────────────────────────────────

int              tollvm_isEOF(void);

// ── Secure Input ────────────────────────────────────────────────────────────

tollvm_io_string tollvm_readPassword(void);
tollvm_io_string tollvm_promptPassword(const char* msg, size_t msgLen);

// ── Stream Control ──────────────────────────────────────────────────────────

void             tollvm_flush(void);
void             tollvm_flushErr(void);

// ── Terminal Detection ──────────────────────────────────────────────────────

int              tollvm_isTTY(void);
int              tollvm_isStdoutTTY(void);
int              tollvm_isStderrTTY(void);

// ── Vec-returning functions ─────────────────────────────────────────────────
// Forward declare vec header (defined in collections/vec.h)
typedef struct { void* ptr; size_t len; size_t cap; } tollvm_io_vec_header;

void tollvm_readLines(tollvm_io_vec_header* out);
void tollvm_readNBytes(tollvm_io_vec_header* out, size_t n);

#ifdef __cplusplus
}
#endif
