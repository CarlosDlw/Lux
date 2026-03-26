#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── String return type (matches tollvm string ABI) ──────────────────────────
typedef struct { const char* ptr; size_t len; } tollvm_str_result;

// ── Search & Match ──────────────────────────────────────────────────────────

int      tollvm_contains(const char* s, size_t sLen,
                         const char* sub, size_t subLen);
int      tollvm_startsWith(const char* s, size_t sLen,
                           const char* prefix, size_t prefixLen);
int      tollvm_endsWith(const char* s, size_t sLen,
                         const char* suffix, size_t suffixLen);
int64_t  tollvm_indexOf(const char* s, size_t sLen,
                        const char* sub, size_t subLen);
int64_t  tollvm_lastIndexOf(const char* s, size_t sLen,
                            const char* sub, size_t subLen);
size_t   tollvm_count(const char* s, size_t sLen,
                      const char* sub, size_t subLen);

// ── Transformation ──────────────────────────────────────────────────────────

tollvm_str_result tollvm_toUpper(const char* s, size_t sLen);
tollvm_str_result tollvm_toLower(const char* s, size_t sLen);
tollvm_str_result tollvm_trim(const char* s, size_t sLen);
tollvm_str_result tollvm_trimLeft(const char* s, size_t sLen);
tollvm_str_result tollvm_trimRight(const char* s, size_t sLen);
tollvm_str_result tollvm_replace(const char* s, size_t sLen,
                                 const char* old, size_t oldLen,
                                 const char* rep, size_t repLen);
tollvm_str_result tollvm_replaceFirst(const char* s, size_t sLen,
                                      const char* old, size_t oldLen,
                                      const char* rep, size_t repLen);
tollvm_str_result tollvm_repeat(const char* s, size_t sLen, size_t n);
tollvm_str_result tollvm_reverse(const char* s, size_t sLen);

// ── Formatting ──────────────────────────────────────────────────────────────

tollvm_str_result tollvm_padLeft(const char* s, size_t sLen,
                                 size_t width, char fill);
tollvm_str_result tollvm_padRight(const char* s, size_t sLen,
                                  size_t width, char fill);

// ── Extraction ──────────────────────────────────────────────────────────────

tollvm_str_result tollvm_substring(const char* s, size_t sLen,
                                   size_t start, size_t length);
char              tollvm_charAt(const char* s, size_t sLen, size_t index);
tollvm_str_result tollvm_slice(const char* s, size_t sLen,
                               int64_t start, int64_t end);

// ── Parsing ─────────────────────────────────────────────────────────────────

int64_t  tollvm_parseInt(const char* s, size_t sLen);
int64_t  tollvm_parseIntRadix(const char* s, size_t sLen, uint32_t radix);
double   tollvm_parseFloat(const char* s, size_t sLen);

// ── Conversion ──────────────────────────────────────────────────────────────

char     tollvm_fromCharCode(int32_t code);

// ── Vec-returning functions ─────────────────────────────────────────────────
typedef struct { void* ptr; size_t len; size_t cap; } tollvm_str_vec_header;

void tollvm_split(tollvm_str_vec_header* out,
                  const char* s, size_t sLen,
                  const char* delim, size_t delimLen);
void tollvm_splitN(tollvm_str_vec_header* out,
                   const char* s, size_t sLen,
                   const char* delim, size_t delimLen, size_t maxParts);
tollvm_str_result tollvm_joinVec(const tollvm_str_vec_header* vec,
                                 const char* sep, size_t sepLen);
void tollvm_lines(tollvm_str_vec_header* out, const char* s, size_t sLen);
void tollvm_chars(tollvm_str_vec_header* out, const char* s, size_t sLen);
tollvm_str_result tollvm_fromCharsVec(const tollvm_str_vec_header* vec);
void tollvm_toBytes(tollvm_str_vec_header* out, const char* s, size_t sLen);
tollvm_str_result tollvm_fromBytesVec(const tollvm_str_vec_header* vec);

// ── C FFI String Conversion ─────────────────────────────────────────────────

char*             tollvm_cstr(const char* s, size_t sLen);
tollvm_str_result  tollvm_fromCStr(const char* cstr);
tollvm_str_result  tollvm_fromCStrLen(const char* cstr, size_t len);

#ifdef __cplusplus
}
#endif
