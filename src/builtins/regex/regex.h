#ifndef LUX_REGEX_H
#define LUX_REGEX_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_regex_str_result;

/* Check if string matches pattern (full or partial match) */
int32_t lux_regexMatch(const char* text, size_t text_len,
                           const char* pat, size_t pat_len);

/* Find first match, return matched substring */
lux_regex_str_result lux_regexFind(const char* text, size_t text_len,
                                          const char* pat, size_t pat_len);

/* Index of first match (-1 if none) */
int64_t lux_regexFindIndex(const char* text, size_t text_len,
                               const char* pat, size_t pat_len);

/* Replace all matches with replacement string */
lux_regex_str_result lux_regexReplace(const char* text, size_t text_len,
                                             const char* pat, size_t pat_len,
                                             const char* rep, size_t rep_len);

/* Replace first match only */
lux_regex_str_result lux_regexReplaceFirst(const char* text, size_t text_len,
                                                  const char* pat, size_t pat_len,
                                                  const char* rep, size_t rep_len);

/* Check if pattern is valid regex */
int32_t lux_regexIsValid(const char* pat, size_t pat_len);

/* Vec-returning regex functions */
typedef struct { void* ptr; size_t len; size_t cap; } lux_regex_vec_header;

void lux_regexFindAll(lux_regex_vec_header* out,
                         const char* text, size_t text_len,
                         const char* pat, size_t pat_len);
void lux_regexSplit(lux_regex_vec_header* out,
                       const char* text, size_t text_len,
                       const char* pat, size_t pat_len);

#ifdef __cplusplus
}
#endif

#endif /* LUX_REGEX_H */
