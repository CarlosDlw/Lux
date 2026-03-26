#ifndef LUX_CONV_H
#define LUX_CONV_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} lux_conv_str_result;

/* Integer → string */
lux_conv_str_result lux_itoa(int64_t value);
lux_conv_str_result lux_itoaRadix(int64_t value, uint32_t radix);
lux_conv_str_result lux_utoa(uint64_t value);

/* Float → string */
lux_conv_str_result lux_ftoa(double value);
lux_conv_str_result lux_ftoaPrecision(double value, uint32_t precision);

/* String → number */
int64_t lux_atoi(const char* data, size_t len);
double  lux_atof(const char* data, size_t len);

/* Radix formatting */
lux_conv_str_result lux_toHex(uint64_t value);
lux_conv_str_result lux_toOctal(uint64_t value);
lux_conv_str_result lux_toBinary(uint64_t value);
uint64_t lux_fromHex(const char* data, size_t len);

/* Char ↔ int */
int32_t lux_charToInt(int8_t c);
int8_t  lux_intToChar(int32_t code);

#ifdef __cplusplus
}
#endif

#endif /* LUX_CONV_H */
