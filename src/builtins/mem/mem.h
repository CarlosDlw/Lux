#ifndef LUX_MEM_H
#define LUX_MEM_H

#include <stddef.h>
#include <stdint.h>

// alloc(usize) -> *void
void* lux_alloc(size_t size);

// allocZeroed(usize) -> *void
void* lux_allocZeroed(size_t size);

// realloc(*void, usize) -> *void
void* lux_realloc(void* ptr, size_t size);

// free(*void)
void lux_free(void* ptr);

// copy(*void, *void, usize)  — dst, src, n
void lux_copy(void* dst, const void* src, size_t n);

// move(*void, *void, usize)  — dst, src, n (overlap-safe)
void lux_move(void* dst, const void* src, size_t n);

// set(*void, uint8, usize)
void lux_set(void* ptr, uint8_t value, size_t n);

// zero(*void, usize)
void lux_zero(void* ptr, size_t n);

// compare(*void, *void, usize) -> int32
int32_t lux_compare(const void* a, const void* b, size_t n);

#endif // LUX_MEM_H
