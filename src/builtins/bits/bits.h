#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// std::bits — Bit manipulation utilities
// ═══════════════════════════════════════════════════════════════════════════════

// Counting
uint32_t lux_popcount(uint64_t v);
uint32_t lux_ctz(uint64_t v);
uint32_t lux_clz(uint64_t v);

// Rotation
uint64_t lux_rotl(uint64_t v, uint32_t n);
uint64_t lux_rotr(uint64_t v, uint32_t n);

// Byte / bit reordering
uint64_t lux_bswap(uint64_t v);
uint64_t lux_bitReverse(uint64_t v);

// Power-of-two checks
int      lux_isPow2(uint64_t v);
uint64_t lux_nextPow2(uint64_t v);

// Bit field operations
uint64_t lux_extractBits(uint64_t v, uint32_t pos, uint32_t width);
uint64_t lux_setBit(uint64_t v, uint32_t pos);
uint64_t lux_clearBit(uint64_t v, uint32_t pos);
uint64_t lux_toggleBit(uint64_t v, uint32_t pos);
int      lux_testBit(uint64_t v, uint32_t pos);

#ifdef __cplusplus
}
#endif
