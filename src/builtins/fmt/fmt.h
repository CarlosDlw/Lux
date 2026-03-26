#ifndef TOLLVM_FMT_H
#define TOLLVM_FMT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_fmt_str_result;

/* Padding */
tollvm_fmt_str_result tollvm_lpad(const char* s, size_t s_len,
                                   size_t width, uint8_t fill);
tollvm_fmt_str_result tollvm_rpad(const char* s, size_t s_len,
                                   size_t width, uint8_t fill);
tollvm_fmt_str_result tollvm_center(const char* s, size_t s_len,
                                     size_t width, uint8_t fill);

/* Integer formatting */
tollvm_fmt_str_result tollvm_fmtHex(uint64_t val);
tollvm_fmt_str_result tollvm_fmtHexUpper(uint64_t val);
tollvm_fmt_str_result tollvm_fmtOct(uint64_t val);
tollvm_fmt_str_result tollvm_fmtBin(uint64_t val);

/* Float formatting */
tollvm_fmt_str_result tollvm_fixed(double val, uint32_t decimals);
tollvm_fmt_str_result tollvm_scientific(double val);

/* Human-readable */
tollvm_fmt_str_result tollvm_humanBytes(uint64_t bytes);
tollvm_fmt_str_result tollvm_commas(int64_t val);
tollvm_fmt_str_result tollvm_percent(double val);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_FMT_H */
