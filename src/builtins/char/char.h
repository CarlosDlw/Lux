#ifndef TOLLVM_CHAR_H
#define TOLLVM_CHAR_H

#include <stdint.h>

// ═══════════════════════════════════════════════════════════════════════════
// std::char — Character Classification & Conversion
//
// All classification functions take uint8_t (char = i8) and return int (bool).
// Conversion functions: toUpper/toLower return uint8_t, toDigit returns int32_t.
// ═══════════════════════════════════════════════════════════════════════════

// Classification
int     tollvm_isAlpha(uint8_t c);
int     tollvm_isDigit(uint8_t c);
int     tollvm_isAlphaNum(uint8_t c);
int     tollvm_isUpper(uint8_t c);
int     tollvm_isLower(uint8_t c);
int     tollvm_isWhitespace(uint8_t c);
int     tollvm_isPrintable(uint8_t c);
int     tollvm_isControl(uint8_t c);
int     tollvm_isHexDigit(uint8_t c);
int     tollvm_isAscii(uint8_t c);

// Conversion
uint8_t tollvm_char_toUpper(uint8_t c);
uint8_t tollvm_char_toLower(uint8_t c);
int32_t tollvm_toDigit(uint8_t c);
uint8_t tollvm_fromDigit(int32_t d);

#endif // TOLLVM_CHAR_H
