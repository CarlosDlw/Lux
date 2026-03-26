#ifndef LUX_FMT_H
#define LUX_FMT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_fmt_str_result;

/* Padding */
lux_fmt_str_result lux_lpad(const char* s, size_t s_len,
                                   size_t width, uint8_t fill);
lux_fmt_str_result lux_rpad(const char* s, size_t s_len,
                                   size_t width, uint8_t fill);
lux_fmt_str_result lux_center(const char* s, size_t s_len,
                                     size_t width, uint8_t fill);

/* Integer formatting */
lux_fmt_str_result lux_fmtHex(uint64_t val);
lux_fmt_str_result lux_fmtHexUpper(uint64_t val);
lux_fmt_str_result lux_fmtOct(uint64_t val);
lux_fmt_str_result lux_fmtBin(uint64_t val);

/* Float formatting */
lux_fmt_str_result lux_fixed(double val, uint32_t decimals);
lux_fmt_str_result lux_scientific(double val);

/* Human-readable */
lux_fmt_str_result lux_humanBytes(uint64_t bytes);
lux_fmt_str_result lux_commas(int64_t val);
lux_fmt_str_result lux_percent(double val);

#ifdef __cplusplus
}
#endif

#endif /* LUX_FMT_H */
