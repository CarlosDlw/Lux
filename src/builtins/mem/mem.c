#include "mem/mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void* tollvm_alloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr && size > 0) {
        fprintf(stderr, "tollvm: alloc failed for %zu bytes\n", size);
        exit(1);
    }
    return ptr;
}

void* tollvm_allocZeroed(size_t size) {
    void* ptr = calloc(1, size);
    if (!ptr && size > 0) {
        fprintf(stderr, "tollvm: allocZeroed failed for %zu bytes\n", size);
        exit(1);
    }
    return ptr;
}

void* tollvm_realloc(void* ptr, size_t size) {
    void* newPtr = realloc(ptr, size);
    if (!newPtr && size > 0) {
        fprintf(stderr, "tollvm: realloc failed for %zu bytes\n", size);
        exit(1);
    }
    return newPtr;
}

void tollvm_free(void* ptr) {
    free(ptr);
}

void tollvm_copy(void* dst, const void* src, size_t n) {
    if (n > 0 && dst && src) {
        memcpy(dst, src, n);
    }
}

void tollvm_move(void* dst, const void* src, size_t n) {
    if (n > 0 && dst && src) {
        memmove(dst, src, n);
    }
}

void tollvm_set(void* ptr, uint8_t value, size_t n) {
    if (n > 0 && ptr) {
        memset(ptr, value, n);
    }
}

void tollvm_zero(void* ptr, size_t n) {
    if (n > 0 && ptr) {
        memset(ptr, 0, n);
    }
}

int32_t tollvm_compare(const void* a, const void* b, size_t n) {
    if (n == 0) return 0;
    if (!a || !b) return a == b ? 0 : (a ? 1 : -1);
    return (int32_t)memcmp(a, b, n);
}
