#ifndef TOLLVM_MEM_H
#define TOLLVM_MEM_H

#include <stddef.h>
#include <stdint.h>

// alloc(usize) -> *void
void* tollvm_alloc(size_t size);

// allocZeroed(usize) -> *void
void* tollvm_allocZeroed(size_t size);

// realloc(*void, usize) -> *void
void* tollvm_realloc(void* ptr, size_t size);

// free(*void)
void tollvm_free(void* ptr);

// copy(*void, *void, usize)  — dst, src, n
void tollvm_copy(void* dst, const void* src, size_t n);

// move(*void, *void, usize)  — dst, src, n (overlap-safe)
void tollvm_move(void* dst, const void* src, size_t n);

// set(*void, uint8, usize)
void tollvm_set(void* ptr, uint8_t value, size_t n);

// zero(*void, usize)
void tollvm_zero(void* ptr, size_t n);

// compare(*void, *void, usize) -> int32
int32_t tollvm_compare(const void* a, const void* b, size_t n);

#endif // TOLLVM_MEM_H
