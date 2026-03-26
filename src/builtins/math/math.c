#include "math.h"

#include <math.h>
#include <stdlib.h>
#include <float.h>

// ── Power & Roots ───────────────────────────────────────────────────────────

double lux_sqrt(double x)                { return sqrt(x); }
double lux_cbrt(double x)                { return cbrt(x); }
double lux_pow(double base, double exp)  { return pow(base, exp); }
double lux_hypot(double a, double b)     { return hypot(a, b); }

// ── Exponential & Log ───────────────────────────────────────────────────────

double lux_exp(double x)   { return exp(x); }
double lux_exp2(double x)  { return exp2(x); }
double lux_ln(double x)    { return log(x); }
double lux_log2(double x)  { return log2(x); }
double lux_log10(double x) { return log10(x); }

// ── Trigonometry ────────────────────────────────────────────────────────────

double lux_sin(double x)             { return sin(x); }
double lux_cos(double x)             { return cos(x); }
double lux_tan(double x)             { return tan(x); }
double lux_asin(double x)            { return asin(x); }
double lux_acos(double x)            { return acos(x); }
double lux_atan(double x)            { return atan(x); }
double lux_atan2(double y, double x) { return atan2(y, x); }

// ── Hyperbolic ──────────────────────────────────────────────────────────────

double lux_sinh(double x) { return sinh(x); }
double lux_cosh(double x) { return cosh(x); }
double lux_tanh(double x) { return tanh(x); }

// ── Rounding ────────────────────────────────────────────────────────────────

double lux_ceil(double x)  { return ceil(x); }
double lux_floor(double x) { return floor(x); }
double lux_round(double x) { return round(x); }
double lux_trunc(double x) { return trunc(x); }

// ── Interpolation ───────────────────────────────────────────────────────────

double lux_lerp(double a, double b, double t) {
    return a + t * (b - a);
}

double lux_map(double val, double inMin, double inMax,
                  double outMin, double outMax) {
    return outMin + (val - inMin) * (outMax - outMin) / (inMax - inMin);
}

// ── Conversion ──────────────────────────────────────────────────────────────

static const double LUX_PI = 3.14159265358979323846;

double lux_toRadians(double deg) { return deg * (LUX_PI / 180.0); }
double lux_toDegrees(double rad) { return rad * (180.0 / LUX_PI); }

// ── Checks ──────────────────────────────────────────────────────────────────

int lux_isNaN(double x)    { return isnan(x) ? 1 : 0; }
int lux_isInf(double x)    { return isinf(x) ? 1 : 0; }
int lux_isFinite(double x) { return isfinite(x) ? 1 : 0; }

// ── Polymorphic: abs ────────────────────────────────────────────────────────

int32_t  lux_abs_i32(int32_t x)  { return x < 0 ? -x : x; }
int64_t  lux_abs_i64(int64_t x)  { return x < 0 ? -x : x; }
float    lux_abs_f32(float x)    { return fabsf(x); }
double   lux_abs_f64(double x)   { return fabs(x); }

// ── Polymorphic: min ────────────────────────────────────────────────────────

int32_t  lux_min_i32(int32_t a, int32_t b)   { return a < b ? a : b; }
int64_t  lux_min_i64(int64_t a, int64_t b)   { return a < b ? a : b; }
uint32_t lux_min_u32(uint32_t a, uint32_t b) { return a < b ? a : b; }
uint64_t lux_min_u64(uint64_t a, uint64_t b) { return a < b ? a : b; }
float    lux_min_f32(float a, float b)       { return a < b ? a : b; }
double   lux_min_f64(double a, double b)     { return a < b ? a : b; }

// ── Polymorphic: max ────────────────────────────────────────────────────────

int32_t  lux_max_i32(int32_t a, int32_t b)   { return a > b ? a : b; }
int64_t  lux_max_i64(int64_t a, int64_t b)   { return a > b ? a : b; }
uint32_t lux_max_u32(uint32_t a, uint32_t b) { return a > b ? a : b; }
uint64_t lux_max_u64(uint64_t a, uint64_t b) { return a > b ? a : b; }
float    lux_max_f32(float a, float b)       { return a > b ? a : b; }
double   lux_max_f64(double a, double b)     { return a > b ? a : b; }

// ── Polymorphic: clamp ──────────────────────────────────────────────────────

int32_t  lux_clamp_i32(int32_t v, int32_t lo, int32_t hi)   { return v < lo ? lo : (v > hi ? hi : v); }
int64_t  lux_clamp_i64(int64_t v, int64_t lo, int64_t hi)   { return v < lo ? lo : (v > hi ? hi : v); }
uint32_t lux_clamp_u32(uint32_t v, uint32_t lo, uint32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }
uint64_t lux_clamp_u64(uint64_t v, uint64_t lo, uint64_t hi) { return v < lo ? lo : (v > hi ? hi : v); }
float    lux_clamp_f32(float v, float lo, float hi)         { return v < lo ? lo : (v > hi ? hi : v); }
double   lux_clamp_f64(double v, double lo, double hi)      { return v < lo ? lo : (v > hi ? hi : v); }
