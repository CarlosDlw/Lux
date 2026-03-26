#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── Power & Roots ───────────────────────────────────────────────────────────

double lux_sqrt(double x);
double lux_cbrt(double x);
double lux_pow(double base, double exp);
double lux_hypot(double a, double b);

// ── Exponential & Log ───────────────────────────────────────────────────────

double lux_exp(double x);
double lux_exp2(double x);
double lux_ln(double x);
double lux_log2(double x);
double lux_log10(double x);

// ── Trigonometry ────────────────────────────────────────────────────────────

double lux_sin(double x);
double lux_cos(double x);
double lux_tan(double x);
double lux_asin(double x);
double lux_acos(double x);
double lux_atan(double x);
double lux_atan2(double y, double x);

// ── Hyperbolic ──────────────────────────────────────────────────────────────

double lux_sinh(double x);
double lux_cosh(double x);
double lux_tanh(double x);

// ── Rounding ────────────────────────────────────────────────────────────────

double lux_ceil(double x);
double lux_floor(double x);
double lux_round(double x);
double lux_trunc(double x);

// ── Interpolation ───────────────────────────────────────────────────────────

double lux_lerp(double a, double b, double t);
double lux_map(double val, double inMin, double inMax,
                  double outMin, double outMax);

// ── Conversion ──────────────────────────────────────────────────────────────

double lux_toRadians(double deg);
double lux_toDegrees(double rad);

// ── Checks ──────────────────────────────────────────────────────────────────

int lux_isNaN(double x);
int lux_isInf(double x);
int lux_isFinite(double x);

// ── Polymorphic: abs ────────────────────────────────────────────────────────

int32_t  lux_abs_i32(int32_t x);
int64_t  lux_abs_i64(int64_t x);
float    lux_abs_f32(float x);
double   lux_abs_f64(double x);

// ── Polymorphic: min ────────────────────────────────────────────────────────

int32_t  lux_min_i32(int32_t a, int32_t b);
int64_t  lux_min_i64(int64_t a, int64_t b);
uint32_t lux_min_u32(uint32_t a, uint32_t b);
uint64_t lux_min_u64(uint64_t a, uint64_t b);
float    lux_min_f32(float a, float b);
double   lux_min_f64(double a, double b);

// ── Polymorphic: max ────────────────────────────────────────────────────────

int32_t  lux_max_i32(int32_t a, int32_t b);
int64_t  lux_max_i64(int64_t a, int64_t b);
uint32_t lux_max_u32(uint32_t a, uint32_t b);
uint64_t lux_max_u64(uint64_t a, uint64_t b);
float    lux_max_f32(float a, float b);
double   lux_max_f64(double a, double b);

// ── Polymorphic: clamp ──────────────────────────────────────────────────────

int32_t  lux_clamp_i32(int32_t val, int32_t lo, int32_t hi);
int64_t  lux_clamp_i64(int64_t val, int64_t lo, int64_t hi);
uint32_t lux_clamp_u32(uint32_t val, uint32_t lo, uint32_t hi);
uint64_t lux_clamp_u64(uint64_t val, uint64_t lo, uint64_t hi);
float    lux_clamp_f32(float val, float lo, float hi);
double   lux_clamp_f64(double val, double lo, double hi);

#ifdef __cplusplus
}
#endif
