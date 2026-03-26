#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// std::bits — Bit manipulation utilities
// ═══════════════════════════════════════════════════════════════════════════════

// Counting
uint32_t tollvm_popcount(uint64_t v);
uint32_t tollvm_ctz(uint64_t v);
uint32_t tollvm_clz(uint64_t v);

// Rotation
uint64_t tollvm_rotl(uint64_t v, uint32_t n);
uint64_t tollvm_rotr(uint64_t v, uint32_t n);

// Byte / bit reordering
uint64_t tollvm_bswap(uint64_t v);
uint64_t tollvm_bitReverse(uint64_t v);

// Power-of-two checks
int      tollvm_isPow2(uint64_t v);
uint64_t tollvm_nextPow2(uint64_t v);

// Bit field operations
uint64_t tollvm_extractBits(uint64_t v, uint32_t pos, uint32_t width);
uint64_t tollvm_setBit(uint64_t v, uint32_t pos);
uint64_t tollvm_clearBit(uint64_t v, uint32_t pos);
uint64_t tollvm_toggleBit(uint64_t v, uint32_t pos);
int      tollvm_testBit(uint64_t v, uint32_t pos);

#ifdef __cplusplus
}
#endif
