#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── Power & Roots ───────────────────────────────────────────────────────────

double tollvm_sqrt(double x);
double tollvm_cbrt(double x);
double tollvm_pow(double base, double exp);
double tollvm_hypot(double a, double b);

// ── Exponential & Log ───────────────────────────────────────────────────────

double tollvm_exp(double x);
double tollvm_exp2(double x);
double tollvm_ln(double x);
double tollvm_log2(double x);
double tollvm_log10(double x);

// ── Trigonometry ────────────────────────────────────────────────────────────

double tollvm_sin(double x);
double tollvm_cos(double x);
double tollvm_tan(double x);
double tollvm_asin(double x);
double tollvm_acos(double x);
double tollvm_atan(double x);
double tollvm_atan2(double y, double x);

// ── Hyperbolic ──────────────────────────────────────────────────────────────

double tollvm_sinh(double x);
double tollvm_cosh(double x);
double tollvm_tanh(double x);

// ── Rounding ────────────────────────────────────────────────────────────────

double tollvm_ceil(double x);
double tollvm_floor(double x);
double tollvm_round(double x);
double tollvm_trunc(double x);

// ── Interpolation ───────────────────────────────────────────────────────────

double tollvm_lerp(double a, double b, double t);
double tollvm_map(double val, double inMin, double inMax,
                  double outMin, double outMax);

// ── Conversion ──────────────────────────────────────────────────────────────

double tollvm_toRadians(double deg);
double tollvm_toDegrees(double rad);

// ── Checks ──────────────────────────────────────────────────────────────────

int tollvm_isNaN(double x);
int tollvm_isInf(double x);
int tollvm_isFinite(double x);

// ── Polymorphic: abs ────────────────────────────────────────────────────────

int32_t  tollvm_abs_i32(int32_t x);
int64_t  tollvm_abs_i64(int64_t x);
float    tollvm_abs_f32(float x);
double   tollvm_abs_f64(double x);

// ── Polymorphic: min ────────────────────────────────────────────────────────

int32_t  tollvm_min_i32(int32_t a, int32_t b);
int64_t  tollvm_min_i64(int64_t a, int64_t b);
uint32_t tollvm_min_u32(uint32_t a, uint32_t b);
uint64_t tollvm_min_u64(uint64_t a, uint64_t b);
float    tollvm_min_f32(float a, float b);
double   tollvm_min_f64(double a, double b);

// ── Polymorphic: max ────────────────────────────────────────────────────────

int32_t  tollvm_max_i32(int32_t a, int32_t b);
int64_t  tollvm_max_i64(int64_t a, int64_t b);
uint32_t tollvm_max_u32(uint32_t a, uint32_t b);
uint64_t tollvm_max_u64(uint64_t a, uint64_t b);
float    tollvm_max_f32(float a, float b);
double   tollvm_max_f64(double a, double b);

// ── Polymorphic: clamp ──────────────────────────────────────────────────────

int32_t  tollvm_clamp_i32(int32_t val, int32_t lo, int32_t hi);
int64_t  tollvm_clamp_i64(int64_t val, int64_t lo, int64_t hi);
uint32_t tollvm_clamp_u32(uint32_t val, uint32_t lo, uint32_t hi);
uint64_t tollvm_clamp_u64(uint64_t val, uint64_t lo, uint64_t hi);
float    tollvm_clamp_f32(float val, float lo, float hi);
double   tollvm_clamp_f64(double val, double lo, double hi);

#ifdef __cplusplus
}
#endif
