#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── Process Control ─────────────────────────────────────────────────────────

void lux_exit(int code);
void lux_panic(const char* msg, size_t len);
void lux_assert(int cond, const char* file, size_t fileLen, int line);
void lux_assertMsg(int cond, const char* msg, size_t msgLen);
void lux_unreachable(const char* file, size_t fileLen, int line);

// ── Type Conversions (string → T) ──────────────────────────────────────────

int64_t lux_toInt(const char* data, size_t len);
double  lux_toFloat(const char* data, size_t len);
int     lux_toBool(const char* data, size_t len);

// ── toString (T → string) ──────────────────────────────────────────────────
// Returns { ptr, len } matching the lux string ABI.
// The returned pointer is heap-allocated; caller owns the memory.

typedef struct { const char* ptr; size_t len; } lux_string_ret;

lux_string_ret lux_toString_i8(int8_t val);
lux_string_ret lux_toString_i16(int16_t val);
lux_string_ret lux_toString_i32(int32_t val);
lux_string_ret lux_toString_i64(int64_t val);
lux_string_ret lux_toString_u8(uint8_t val);
lux_string_ret lux_toString_u16(uint16_t val);
lux_string_ret lux_toString_u32(uint32_t val);
lux_string_ret lux_toString_u64(uint64_t val);
lux_string_ret lux_toString_f32(float val);
lux_string_ret lux_toString_f64(double val);
lux_string_ret lux_toString_bool(int val);
lux_string_ret lux_toString_char(char val);

#ifdef __cplusplus
}
#endif
