#ifndef TOLLVM_CONV_H
#define TOLLVM_CONV_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_conv_str_result;

/* Integer → string */
tollvm_conv_str_result tollvm_itoa(int64_t value);
tollvm_conv_str_result tollvm_itoaRadix(int64_t value, uint32_t radix);
tollvm_conv_str_result tollvm_utoa(uint64_t value);

/* Float → string */
tollvm_conv_str_result tollvm_ftoa(double value);
tollvm_conv_str_result tollvm_ftoaPrecision(double value, uint32_t precision);

/* String → number */
int64_t tollvm_atoi(const char* data, size_t len);
double  tollvm_atof(const char* data, size_t len);

/* Radix formatting */
tollvm_conv_str_result tollvm_toHex(uint64_t value);
tollvm_conv_str_result tollvm_toOctal(uint64_t value);
tollvm_conv_str_result tollvm_toBinary(uint64_t value);
uint64_t tollvm_fromHex(const char* data, size_t len);

/* Char ↔ int */
int32_t tollvm_charToInt(int8_t c);
int8_t  tollvm_intToChar(int32_t code);

#ifdef __cplusplus
}
#endif

#endif /* TOLLVM_CONV_H */
