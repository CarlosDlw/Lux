#include "math.h"

#include <math.h>
#include <stdlib.h>
#include <float.h>

// ── Power & Roots ───────────────────────────────────────────────────────────

double tollvm_sqrt(double x)                { return sqrt(x); }
double tollvm_cbrt(double x)                { return cbrt(x); }
double tollvm_pow(double base, double exp)  { return pow(base, exp); }
double tollvm_hypot(double a, double b)     { return hypot(a, b); }

// ── Exponential & Log ───────────────────────────────────────────────────────

double tollvm_exp(double x)   { return exp(x); }
double tollvm_exp2(double x)  { return exp2(x); }
double tollvm_ln(double x)    { return log(x); }
double tollvm_log2(double x)  { return log2(x); }
double tollvm_log10(double x) { return log10(x); }

// ── Trigonometry ────────────────────────────────────────────────────────────

double tollvm_sin(double x)             { return sin(x); }
double tollvm_cos(double x)             { return cos(x); }
double tollvm_tan(double x)             { return tan(x); }
double tollvm_asin(double x)            { return asin(x); }
double tollvm_acos(double x)            { return acos(x); }
double tollvm_atan(double x)            { return atan(x); }
double tollvm_atan2(double y, double x) { return atan2(y, x); }

// ── Hyperbolic ──────────────────────────────────────────────────────────────

double tollvm_sinh(double x) { return sinh(x); }
double tollvm_cosh(double x) { return cosh(x); }
double tollvm_tanh(double x) { return tanh(x); }

// ── Rounding ────────────────────────────────────────────────────────────────

double tollvm_ceil(double x)  { return ceil(x); }
double tollvm_floor(double x) { return floor(x); }
double tollvm_round(double x) { return round(x); }
double tollvm_trunc(double x) { return trunc(x); }

// ── Interpolation ───────────────────────────────────────────────────────────

double tollvm_lerp(double a, double b, double t) {
    return a + t * (b - a);
}

double tollvm_map(double val, double inMin, double inMax,
                  double outMin, double outMax) {
    return outMin + (val - inMin) * (outMax - outMin) / (inMax - inMin);
}

// ── Conversion ──────────────────────────────────────────────────────────────

static const double TOLLVM_PI = 3.14159265358979323846;

double tollvm_toRadians(double deg) { return deg * (TOLLVM_PI / 180.0); }
double tollvm_toDegrees(double rad) { return rad * (180.0 / TOLLVM_PI); }

// ── Checks ──────────────────────────────────────────────────────────────────

int tollvm_isNaN(double x)    { return isnan(x) ? 1 : 0; }
int tollvm_isInf(double x)    { return isinf(x) ? 1 : 0; }
int tollvm_isFinite(double x) { return isfinite(x) ? 1 : 0; }

// ── Polymorphic: abs ────────────────────────────────────────────────────────

int32_t  tollvm_abs_i32(int32_t x)  { return x < 0 ? -x : x; }
int64_t  tollvm_abs_i64(int64_t x)  { return x < 0 ? -x : x; }
float    tollvm_abs_f32(float x)    { return fabsf(x); }
double   tollvm_abs_f64(double x)   { return fabs(x); }

// ── Polymorphic: min ────────────────────────────────────────────────────────

int32_t  tollvm_min_i32(int32_t a, int32_t b)   { return a < b ? a : b; }
int64_t  tollvm_min_i64(int64_t a, int64_t b)   { return a < b ? a : b; }
uint32_t tollvm_min_u32(uint32_t a, uint32_t b) { return a < b ? a : b; }
uint64_t tollvm_min_u64(uint64_t a, uint64_t b) { return a < b ? a : b; }
float    tollvm_min_f32(float a, float b)       { return a < b ? a : b; }
double   tollvm_min_f64(double a, double b)     { return a < b ? a : b; }

// ── Polymorphic: max ────────────────────────────────────────────────────────

int32_t  tollvm_max_i32(int32_t a, int32_t b)   { return a > b ? a : b; }
int64_t  tollvm_max_i64(int64_t a, int64_t b)   { return a > b ? a : b; }
uint32_t tollvm_max_u32(uint32_t a, uint32_t b) { return a > b ? a : b; }
uint64_t tollvm_max_u64(uint64_t a, uint64_t b) { return a > b ? a : b; }
float    tollvm_max_f32(float a, float b)       { return a > b ? a : b; }
double   tollvm_max_f64(double a, double b)     { return a > b ? a : b; }

// ── Polymorphic: clamp ──────────────────────────────────────────────────────

int32_t  tollvm_clamp_i32(int32_t v, int32_t lo, int32_t hi)   { return v < lo ? lo : (v > hi ? hi : v); }
int64_t  tollvm_clamp_i64(int64_t v, int64_t lo, int64_t hi)   { return v < lo ? lo : (v > hi ? hi : v); }
uint32_t tollvm_clamp_u32(uint32_t v, uint32_t lo, uint32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }
uint64_t tollvm_clamp_u64(uint64_t v, uint64_t lo, uint64_t hi) { return v < lo ? lo : (v > hi ? hi : v); }
float    tollvm_clamp_f32(float v, float lo, float hi)         { return v < lo ? lo : (v > hi ? hi : v); }
double   tollvm_clamp_f64(double v, double lo, double hi)      { return v < lo ? lo : (v > hi ? hi : v); }
