#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── Process Control ─────────────────────────────────────────────────────────

void tollvm_exit(int code);
void tollvm_panic(const char* msg, size_t len);
void tollvm_assert(int cond, const char* file, size_t fileLen, int line);
void tollvm_assertMsg(int cond, const char* msg, size_t msgLen);
void tollvm_unreachable(const char* file, size_t fileLen, int line);

// ── Type Conversions (string → T) ──────────────────────────────────────────

int64_t tollvm_toInt(const char* data, size_t len);
double  tollvm_toFloat(const char* data, size_t len);
int     tollvm_toBool(const char* data, size_t len);

// ── toString (T → string) ──────────────────────────────────────────────────
// Returns { ptr, len } matching the tollvm string ABI.
// The returned pointer is heap-allocated; caller owns the memory.

typedef struct { const char* ptr; size_t len; } tollvm_string_ret;

tollvm_string_ret tollvm_toString_i8(int8_t val);
tollvm_string_ret tollvm_toString_i16(int16_t val);
tollvm_string_ret tollvm_toString_i32(int32_t val);
tollvm_string_ret tollvm_toString_i64(int64_t val);
tollvm_string_ret tollvm_toString_u8(uint8_t val);
tollvm_string_ret tollvm_toString_u16(uint16_t val);
tollvm_string_ret tollvm_toString_u32(uint32_t val);
tollvm_string_ret tollvm_toString_u64(uint64_t val);
tollvm_string_ret tollvm_toString_f32(float val);
tollvm_string_ret tollvm_toString_f64(double val);
tollvm_string_ret tollvm_toString_bool(int val);
tollvm_string_ret tollvm_toString_char(char val);

#ifdef __cplusplus
}
#endif
