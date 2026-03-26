#ifndef LUX_TEST_H
#define LUX_TEST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* assertEqual variants */
void lux_assertEqualI64(int64_t a, int64_t b);
void lux_assertEqualF64(double a, double b);
void lux_assertEqualStr(const char* a, size_t alen,
                           const char* b, size_t blen);
void lux_assertEqualBool(int32_t a, int32_t b);
void lux_assertEqualChar(int8_t a, int8_t b);

/* assertNotEqual variants */
void lux_assertNotEqualI64(int64_t a, int64_t b);
void lux_assertNotEqualF64(double a, double b);
void lux_assertNotEqualStr(const char* a, size_t alen,
                              const char* b, size_t blen);
void lux_assertNotEqualBool(int32_t a, int32_t b);
void lux_assertNotEqualChar(int8_t a, int8_t b);

/* assertTrue / assertFalse */
void lux_assertTrue(int32_t cond);
void lux_assertFalse(int32_t cond);

/* Ordered comparisons (numeric only) */
void lux_assertGreaterI64(int64_t a, int64_t b);
void lux_assertGreaterF64(double a, double b);
void lux_assertLessI64(int64_t a, int64_t b);
void lux_assertLessF64(double a, double b);
void lux_assertGreaterEqI64(int64_t a, int64_t b);
void lux_assertGreaterEqF64(double a, double b);
void lux_assertLessEqI64(int64_t a, int64_t b);
void lux_assertLessEqF64(double a, double b);

/* String-specific */
void lux_assertStringContains(const char* s, size_t slen,
                                 const char* sub, size_t sublen);

/* Float near-equality */
void lux_assertNear(double a, double b, double epsilon);

/* Utilities */
void lux_testFail(const char* msg, size_t msglen);
void lux_testSkip(const char* msg, size_t msglen);
void lux_testLog(const char* msg, size_t msglen);

#ifdef __cplusplus
}
#endif

#endif /* LUX_TEST_H */
