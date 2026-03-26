#ifndef TOLLVM_TEST_H
#define TOLLVM_TEST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* assertEqual variants */
void tollvm_assertEqualI64(int64_t a, int64_t b);
void tollvm_assertEqualF64(double a, double b);
void tollvm_assertEqualStr(const char* a, size_t alen,
                           const char* b, size_t blen);
void tollvm_assertEqualBool(int32_t a, int32_t b);
void tollvm_assertEqualChar(int8_t a, int8_t b);

/* assertNotEqual variants */
void tollvm_assertNotEqualI64(int64_t a, int64_t b);
void tollvm_assertNotEqualF64(double a, double b);
void tollvm_assertNotEqualStr(const char* a, size_t alen,
                              const char* b, size_t blen);
void tollvm_assertNotEqualBool(int32_t a, int32_t b);
void tollvm_assertNotEqualChar(int8_t a, int8_t b);

/* assertTrue / assertFalse */
void tollvm_assertTrue(int32_t cond);
void tollvm_assertFalse(int32_t cond);

/* Ordered comparisons (numeric only) */
void tollvm_assertGreaterI64(int64_t a, int64_t b);
void tollvm_assertGreaterF64(double a, double b);
void tollvm_assertLessI64(int64_t a, int64_t b);
void tollvm_assertLessF64(double a, double b);
void tollvm_assertGreaterEqI64(int64_t a, int64_t b);
void tollvm_assertGreaterEqF64(double a, double b);
void tollvm_assertLessEqI64(int64_t a, int64_t b);
void tollvm_assertLessEqF64(double a, double b);

/* String-specific */
void tollvm_assertStringContains(const char* s, size_t slen,
                                 const char* sub, size_t sublen);

/* Float near-equality */
void tollvm_assertNear(double a, double b, double epsilon);

/* Utilities */
void tollvm_testFail(const char* msg, size_t msglen);
void tollvm_testSkip(const char* msg, size_t msglen);
void tollvm_testLog(const char* msg, size_t msglen);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_TEST_H */
