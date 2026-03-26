#ifndef TOLLVM_REGEX_H
#define TOLLVM_REGEX_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_regex_str_result;

/* Check if string matches pattern (full or partial match) */
int32_t tollvm_regexMatch(const char* text, size_t text_len,
                           const char* pat, size_t pat_len);

/* Find first match, return matched substring */
tollvm_regex_str_result tollvm_regexFind(const char* text, size_t text_len,
                                          const char* pat, size_t pat_len);

/* Index of first match (-1 if none) */
int64_t tollvm_regexFindIndex(const char* text, size_t text_len,
                               const char* pat, size_t pat_len);

/* Replace all matches with replacement string */
tollvm_regex_str_result tollvm_regexReplace(const char* text, size_t text_len,
                                             const char* pat, size_t pat_len,
                                             const char* rep, size_t rep_len);

/* Replace first match only */
tollvm_regex_str_result tollvm_regexReplaceFirst(const char* text, size_t text_len,
                                                  const char* pat, size_t pat_len,
                                                  const char* rep, size_t rep_len);

/* Check if pattern is valid regex */
int32_t tollvm_regexIsValid(const char* pat, size_t pat_len);

/* Vec-returning regex functions */
typedef struct { void* ptr; size_t len; size_t cap; } tollvm_regex_vec_header;

void tollvm_regexFindAll(tollvm_regex_vec_header* out,
                         const char* text, size_t text_len,
                         const char* pat, size_t pat_len);
void tollvm_regexSplit(tollvm_regex_vec_header* out,
                       const char* text, size_t text_len,
                       const char* pat, size_t pat_len);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_REGEX_H */
