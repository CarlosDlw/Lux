#ifndef LUX_CHAR_H
#define LUX_CHAR_H

#include <stdint.h>

// ═══════════════════════════════════════════════════════════════════════════
// std::char — Character Classification & Conversion
//
// All classification functions take uint8_t (char = i8) and return int (bool).
// Conversion functions: toUpper/toLower return uint8_t, toDigit returns int32_t.
// ═══════════════════════════════════════════════════════════════════════════

// Classification
int     lux_isAlpha(uint8_t c);
int     lux_isDigit(uint8_t c);
int     lux_isAlphaNum(uint8_t c);
int     lux_isUpper(uint8_t c);
int     lux_isLower(uint8_t c);
int     lux_isWhitespace(uint8_t c);
int     lux_isPrintable(uint8_t c);
int     lux_isControl(uint8_t c);
int     lux_isHexDigit(uint8_t c);
int     lux_isAscii(uint8_t c);

// Conversion
uint8_t lux_char_toUpper(uint8_t c);
uint8_t lux_char_toLower(uint8_t c);
int32_t lux_toDigit(uint8_t c);
uint8_t lux_fromDigit(int32_t d);

#endif // LUX_CHAR_H
