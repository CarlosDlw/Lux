#include "vec.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ═══════════════════════════════════════════════════════════════════════════════
// vec<T> runtime — macro-generated monomorphized implementations
// ═══════════════════════════════════════════════════════════════════════════════

#define TOLLVM_VEC_INITIAL_CAP 8
#define TOLLVM_VEC_GROWTH_FACTOR 2

#define TOLLVM_VEC_IMPL(T, SUFFIX)                                              \
                                                                                \
/* ── Typed data pointer helper ─────────────────────────────────────────── */  \
static inline T* vec_data_##SUFFIX(tollvm_vec_header* v) {                      \
    return (T*)v->ptr;                                                          \
}                                                                               \
static inline const T* vec_cdata_##SUFFIX(const tollvm_vec_header* v) {         \
    return (const T*)v->ptr;                                                    \
}                                                                               \
                                                                                \
/* ── Ensure capacity ───────────────────────────────────────────────────── */  \
static void vec_grow_##SUFFIX(tollvm_vec_header* v, size_t needed) {            \
    if (needed <= v->cap) return;                                               \
    size_t newCap = v->cap ? v->cap : TOLLVM_VEC_INITIAL_CAP;                   \
    while (newCap < needed) newCap *= TOLLVM_VEC_GROWTH_FACTOR;                 \
    v->ptr = realloc(v->ptr, newCap * sizeof(T));                               \
    v->cap = newCap;                                                            \
}                                                                               \
                                                                                \
/* ── Creation / destruction ────────────────────────────────────────────── */  \
void tollvm_vec_init_##SUFFIX(tollvm_vec_header* v) {                           \
    v->ptr = NULL;                                                              \
    v->len = 0;                                                                 \
    v->cap = 0;                                                                 \
}                                                                               \
                                                                                \
void tollvm_vec_init_cap_##SUFFIX(tollvm_vec_header* v, size_t cap) {           \
    v->ptr = malloc(cap * sizeof(T));                                           \
    v->len = 0;                                                                 \
    v->cap = cap;                                                               \
}                                                                               \
                                                                                \
void tollvm_vec_free_##SUFFIX(tollvm_vec_header* v) {                           \
    free(v->ptr);                                                               \
    v->ptr = NULL;                                                              \
    v->len = 0;                                                                 \
    v->cap = 0;                                                                 \
}                                                                               \
                                                                                \
/* ── Size / capacity ───────────────────────────────────────────────────── */  \
size_t tollvm_vec_len_##SUFFIX(const tollvm_vec_header* v) {                    \
    return v->len;                                                              \
}                                                                               \
                                                                                \
size_t tollvm_vec_capacity_##SUFFIX(const tollvm_vec_header* v) {               \
    return v->cap;                                                              \
}                                                                               \
                                                                                \
int tollvm_vec_isEmpty_##SUFFIX(const tollvm_vec_header* v) {                   \
    return v->len == 0;                                                         \
}                                                                               \
                                                                                \
/* ── Element access ────────────────────────────────────────────────────── */  \
T tollvm_vec_at_##SUFFIX(const tollvm_vec_header* v, size_t idx) {              \
    if (idx >= v->len) {                                                        \
        fprintf(stderr, "tollvm: vec index out of bounds: %zu >= %zu\n",        \
                idx, v->len);                                                   \
        exit(1);                                                                \
    }                                                                           \
    return vec_cdata_##SUFFIX(v)[idx];                                          \
}                                                                               \
                                                                                \
T tollvm_vec_first_##SUFFIX(const tollvm_vec_header* v) {                       \
    if (v->len == 0) {                                                          \
        fprintf(stderr, "tollvm: vec.first() on empty vec\n");                  \
        exit(1);                                                                \
    }                                                                           \
    return vec_cdata_##SUFFIX(v)[0];                                            \
}                                                                               \
                                                                                \
T tollvm_vec_last_##SUFFIX(const tollvm_vec_header* v) {                        \
    if (v->len == 0) {                                                          \
        fprintf(stderr, "tollvm: vec.last() on empty vec\n");                   \
        exit(1);                                                                \
    }                                                                           \
    return vec_cdata_##SUFFIX(v)[v->len - 1];                                   \
}                                                                               \
                                                                                \
void tollvm_vec_set_##SUFFIX(tollvm_vec_header* v, size_t idx, T val) {       \
    if (idx >= v->len) {                                                        \
        fprintf(stderr, "tollvm: vec index out of bounds: %zu >= %zu\n",        \
                idx, v->len);                                                   \
        exit(1);                                                                \
    }                                                                           \
    vec_data_##SUFFIX(v)[idx] = val;                                            \
}                                                                               \
                                                                                \
/* ── Mutation ──────────────────────────────────────────────────────────── */  \
void tollvm_vec_push_##SUFFIX(tollvm_vec_header* v, T val) {                    \
    vec_grow_##SUFFIX(v, v->len + 1);                                           \
    vec_data_##SUFFIX(v)[v->len++] = val;                                       \
}                                                                               \
                                                                                \
T tollvm_vec_pop_##SUFFIX(tollvm_vec_header* v) {                               \
    if (v->len == 0) {                                                          \
        fprintf(stderr, "tollvm: vec.pop() on empty vec\n");                    \
        exit(1);                                                                \
    }                                                                           \
    return vec_data_##SUFFIX(v)[--v->len];                                      \
}                                                                               \
                                                                                \
void tollvm_vec_insert_##SUFFIX(tollvm_vec_header* v, size_t idx, T val) {      \
    if (idx > v->len) {                                                         \
        fprintf(stderr, "tollvm: vec.insert() index out of bounds\n");          \
        exit(1);                                                                \
    }                                                                           \
    vec_grow_##SUFFIX(v, v->len + 1);                                           \
    T* d = vec_data_##SUFFIX(v);                                                \
    memmove(&d[idx + 1], &d[idx], (v->len - idx) * sizeof(T));                 \
    d[idx] = val;                                                               \
    v->len++;                                                                   \
}                                                                               \
                                                                                \
T tollvm_vec_removeAt_##SUFFIX(tollvm_vec_header* v, size_t idx) {              \
    if (idx >= v->len) {                                                        \
        fprintf(stderr, "tollvm: vec.removeAt() index out of bounds\n");        \
        exit(1);                                                                \
    }                                                                           \
    T* d = vec_data_##SUFFIX(v);                                                \
    T val = d[idx];                                                             \
    memmove(&d[idx], &d[idx + 1], (v->len - idx - 1) * sizeof(T));             \
    v->len--;                                                                   \
    return val;                                                                 \
}                                                                               \
                                                                                \
T tollvm_vec_removeSwap_##SUFFIX(tollvm_vec_header* v, size_t idx) {            \
    if (idx >= v->len) {                                                        \
        fprintf(stderr, "tollvm: vec.removeSwap() index out of bounds\n");      \
        exit(1);                                                                \
    }                                                                           \
    T* d = vec_data_##SUFFIX(v);                                                \
    T val = d[idx];                                                             \
    d[idx] = d[v->len - 1];                                                     \
    v->len--;                                                                   \
    return val;                                                                 \
}                                                                               \
                                                                                \
void tollvm_vec_clear_##SUFFIX(tollvm_vec_header* v) {                          \
    v->len = 0;                                                                 \
}                                                                               \
                                                                                \
void tollvm_vec_fill_##SUFFIX(tollvm_vec_header* v, T val) {                    \
    T* d = vec_data_##SUFFIX(v);                                                \
    for (size_t i = 0; i < v->len; i++) d[i] = val;                            \
}                                                                               \
                                                                                \
void tollvm_vec_swap_##SUFFIX(tollvm_vec_header* v, size_t i, size_t j) {       \
    if (i >= v->len || j >= v->len) {                                           \
        fprintf(stderr, "tollvm: vec.swap() index out of bounds\n");            \
        exit(1);                                                                \
    }                                                                           \
    T* d = vec_data_##SUFFIX(v);                                                \
    T tmp = d[i]; d[i] = d[j]; d[j] = tmp;                                     \
}                                                                               \
                                                                                \
/* ── Memory management ─────────────────────────────────────────────────── */  \
void tollvm_vec_reserve_##SUFFIX(tollvm_vec_header* v, size_t cap) {            \
    vec_grow_##SUFFIX(v, cap);                                                  \
}                                                                               \
                                                                                \
void tollvm_vec_shrink_##SUFFIX(tollvm_vec_header* v) {                         \
    if (v->len == 0) {                                                          \
        free(v->ptr);                                                           \
        v->ptr = NULL;                                                          \
        v->cap = 0;                                                             \
    } else if (v->cap > v->len) {                                               \
        v->ptr = realloc(v->ptr, v->len * sizeof(T));                           \
        v->cap = v->len;                                                        \
    }                                                                           \
}                                                                               \
                                                                                \
void tollvm_vec_resize_##SUFFIX(tollvm_vec_header* v, size_t len, T fill) {     \
    vec_grow_##SUFFIX(v, len);                                                  \
    T* d = vec_data_##SUFFIX(v);                                                \
    for (size_t i = v->len; i < len; i++) d[i] = fill;                         \
    v->len = len;                                                               \
}                                                                               \
                                                                                \
void tollvm_vec_truncate_##SUFFIX(tollvm_vec_header* v, size_t len) {           \
    if (len < v->len) v->len = len;                                             \
}                                                                               \
                                                                                \
/* ── Search ────────────────────────────────────────────────────────────── */  \
int tollvm_vec_contains_##SUFFIX(const tollvm_vec_header* v, T val) {           \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    for (size_t i = 0; i < v->len; i++)                                         \
        if (d[i] == val) return 1;                                              \
    return 0;                                                                   \
}                                                                               \
                                                                                \
long long tollvm_vec_indexOf_##SUFFIX(const tollvm_vec_header* v, T val) {      \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    for (size_t i = 0; i < v->len; i++)                                         \
        if (d[i] == val) return (long long)i;                                   \
    return -1;                                                                  \
}                                                                               \
                                                                                \
long long tollvm_vec_lastIndexOf_##SUFFIX(const tollvm_vec_header* v, T val) {  \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    for (size_t i = v->len; i > 0; i--)                                         \
        if (d[i - 1] == val) return (long long)(i - 1);                         \
    return -1;                                                                  \
}                                                                               \
                                                                                \
size_t tollvm_vec_count_##SUFFIX(const tollvm_vec_header* v, T val) {           \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    size_t c = 0;                                                               \
    for (size_t i = 0; i < v->len; i++)                                         \
        if (d[i] == val) c++;                                                   \
    return c;                                                                   \
}                                                                               \
                                                                                \
/* ── Reorder ───────────────────────────────────────────────────────────── */  \
void tollvm_vec_reverse_##SUFFIX(tollvm_vec_header* v) {                        \
    T* d = vec_data_##SUFFIX(v);                                                \
    for (size_t i = 0, j = v->len - 1; i < j; i++, j--) {                      \
        T tmp = d[i]; d[i] = d[j]; d[j] = tmp;                                 \
    }                                                                           \
}                                                                               \
                                                                                \
/* ── Comparison ────────────────────────────────────────────────────────── */  \
int tollvm_vec_equals_##SUFFIX(const tollvm_vec_header* a,                      \
                               const tollvm_vec_header* b) {                    \
    if (a->len != b->len) return 0;                                             \
    const T* da = vec_cdata_##SUFFIX(a);                                        \
    const T* db = (const T*)b->ptr;                                             \
    for (size_t i = 0; i < a->len; i++)                                         \
        if (da[i] != db[i]) return 0;                                           \
    return 1;                                                                   \
}                                                                               \
                                                                                \
int tollvm_vec_isSorted_##SUFFIX(const tollvm_vec_header* v) {                  \
    if (v->len <= 1) return 1;                                                  \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    for (size_t i = 1; i < v->len; i++)                                         \
        if (d[i] < d[i - 1]) return 0;                                         \
    return 1;                                                                   \
}                                                                               \
                                                                                \
/* ── Sort ──────────────────────────────────────────────────────────────── */  \
static int vec_cmp_asc_##SUFFIX(const void* a, const void* b) {                 \
    T va = *(const T*)a, vb = *(const T*)b;                                     \
    return (va > vb) - (va < vb);                                               \
}                                                                               \
static int vec_cmp_desc_##SUFFIX(const void* a, const void* b) {                \
    T va = *(const T*)a, vb = *(const T*)b;                                     \
    return (vb > va) - (vb < va);                                               \
}                                                                               \
void tollvm_vec_sort_##SUFFIX(tollvm_vec_header* v) {                           \
    if (v->len > 1)                                                             \
        qsort(v->ptr, v->len, sizeof(T), vec_cmp_asc_##SUFFIX);                \
}                                                                               \
void tollvm_vec_sortDesc_##SUFFIX(tollvm_vec_header* v) {                       \
    if (v->len > 1)                                                             \
        qsort(v->ptr, v->len, sizeof(T), vec_cmp_desc_##SUFFIX);               \
}                                                                               \
                                                                                \
/* ── Rotate ────────────────────────────────────────────────────────────── */  \
void tollvm_vec_rotate_##SUFFIX(tollvm_vec_header* v, int32_t steps) {          \
    if (v->len <= 1) return;                                                    \
    int64_t n = (int64_t)v->len;                                                \
    int64_t s = ((int64_t)steps % n + n) % n;                                   \
    if (s == 0) return;                                                         \
    T* d = vec_data_##SUFFIX(v);                                                \
    /* reverse [0..n), then [0..s), then [s..n) */                              \
    for (size_t i = 0, j = v->len - 1; i < j; i++, j--) {                      \
        T tmp = d[i]; d[i] = d[j]; d[j] = tmp; }                               \
    for (size_t i = 0, j = (size_t)s - 1; i < j; i++, j--) {                   \
        T tmp = d[i]; d[i] = d[j]; d[j] = tmp; }                               \
    for (size_t i = (size_t)s, j = v->len - 1; i < j; i++, j--) {              \
        T tmp = d[i]; d[i] = d[j]; d[j] = tmp; }                               \
}                                                                               \
                                                                                \
/* ── Aggregation ───────────────────────────────────────────────────────── */  \
T tollvm_vec_sum_##SUFFIX(const tollvm_vec_header* v) {                         \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    T acc = 0;                                                                  \
    for (size_t i = 0; i < v->len; i++) acc += d[i];                            \
    return acc;                                                                 \
}                                                                               \
T tollvm_vec_product_##SUFFIX(const tollvm_vec_header* v) {                     \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    T acc = 1;                                                                  \
    for (size_t i = 0; i < v->len; i++) acc *= d[i];                            \
    return acc;                                                                 \
}                                                                               \
T tollvm_vec_min_##SUFFIX(const tollvm_vec_header* v) {                         \
    if (v->len == 0) {                                                          \
        fprintf(stderr, "tollvm: vec.min() on empty vec\n"); exit(1); }         \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    T m = d[0];                                                                 \
    for (size_t i = 1; i < v->len; i++) if (d[i] < m) m = d[i];               \
    return m;                                                                   \
}                                                                               \
T tollvm_vec_max_##SUFFIX(const tollvm_vec_header* v) {                         \
    if (v->len == 0) {                                                          \
        fprintf(stderr, "tollvm: vec.max() on empty vec\n"); exit(1); }         \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    T m = d[0];                                                                 \
    for (size_t i = 1; i < v->len; i++) if (d[i] > m) m = d[i];               \
    return m;                                                                   \
}                                                                               \
double tollvm_vec_average_##SUFFIX(const tollvm_vec_header* v) {                \
    if (v->len == 0) return 0.0;                                                \
    const T* d = vec_cdata_##SUFFIX(v);                                         \
    double sum = 0.0;                                                           \
    for (size_t i = 0; i < v->len; i++) sum += (double)d[i];                   \
    return sum / (double)v->len;                                                \
}                                                                               \
                                                                                \
/* ── Clone ─────────────────────────────────────────────────────────────── */  \
void tollvm_vec_clone_##SUFFIX(const tollvm_vec_header* src,                    \
                               tollvm_vec_header* dst) {                        \
    dst->len = src->len;                                                        \
    dst->cap = src->len;                                                        \
    if (src->len > 0) {                                                         \
        dst->ptr = malloc(src->len * sizeof(T));                                \
        memcpy(dst->ptr, src->ptr, src->len * sizeof(T));                       \
    } else {                                                                    \
        dst->ptr = NULL;                                                        \
        dst->cap = 0;                                                           \
    }                                                                           \
}

// ═══════════════════════════════════════════════════════════════════════════════
// Instantiate for all primitive types
// ═══════════════════════════════════════════════════════════════════════════════

TOLLVM_VEC_IMPL(int8_t,    i8)
TOLLVM_VEC_IMPL(int16_t,   i16)
TOLLVM_VEC_IMPL(int32_t,   i32)
TOLLVM_VEC_IMPL(int64_t,   i64)
TOLLVM_VEC_IMPL(uint8_t,   u8)
TOLLVM_VEC_IMPL(uint16_t,  u16)
TOLLVM_VEC_IMPL(uint32_t,  u32)
TOLLVM_VEC_IMPL(uint64_t,  u64)
TOLLVM_VEC_IMPL(float,     f32)
TOLLVM_VEC_IMPL(double,    f64)
TOLLVM_VEC_IMPL(char,      char)

// ═══════════════════════════════════════════════════════════════════════════════
// toString / join — format-specific implementations
// ═══════════════════════════════════════════════════════════════════════════════

// Helper: build a string by iterating elements with a format specifier
#define TOLLVM_VEC_TOSTRING_IMPL(T, SUFFIX, FMT)                                \
tollvm_string tollvm_vec_toString_##SUFFIX(const tollvm_vec_header* v) {         \
    if (v->len == 0) {                                                          \
        const char* s = "[]"; tollvm_string r = { s, 2 }; return r;            \
    }                                                                           \
    size_t cap = 64;                                                            \
    char* buf = (char*)malloc(cap);                                             \
    size_t pos = 0;                                                             \
    buf[pos++] = '[';                                                           \
    const T* d = (const T*)v->ptr;                                              \
    for (size_t i = 0; i < v->len; i++) {                                       \
        if (i > 0) { buf[pos++] = ','; buf[pos++] = ' '; }                     \
        while (cap - pos < 32) { cap *= 2; buf = (char*)realloc(buf, cap); }   \
        pos += (size_t)snprintf(buf + pos, cap - pos, FMT, d[i]);              \
    }                                                                           \
    if (pos + 1 >= cap) { cap = pos + 2; buf = (char*)realloc(buf, cap); }     \
    buf[pos++] = ']';                                                           \
    buf[pos] = '\0';                                                            \
    tollvm_string r = { buf, pos }; return r;                                   \
}                                                                               \
                                                                                \
tollvm_string tollvm_vec_join_##SUFFIX(const tollvm_vec_header* v,              \
                                       tollvm_string sep) {                     \
    if (v->len == 0) { tollvm_string r = { "", 0 }; return r; }                \
    size_t cap = 64;                                                            \
    char* buf = (char*)malloc(cap);                                             \
    size_t pos = 0;                                                             \
    const T* d = (const T*)v->ptr;                                              \
    for (size_t i = 0; i < v->len; i++) {                                       \
        if (i > 0) {                                                            \
            while (cap - pos < sep.len + 1) { cap *= 2; buf = (char*)realloc(buf, cap); } \
            memcpy(buf + pos, sep.ptr, sep.len); pos += sep.len;                \
        }                                                                       \
        while (cap - pos < 32) { cap *= 2; buf = (char*)realloc(buf, cap); }   \
        pos += (size_t)snprintf(buf + pos, cap - pos, FMT, d[i]);              \
    }                                                                           \
    buf[pos] = '\0';                                                            \
    tollvm_string r = { buf, pos }; return r;                                   \
}

TOLLVM_VEC_TOSTRING_IMPL(int8_t,   i8,   "%d")
TOLLVM_VEC_TOSTRING_IMPL(int16_t,  i16,  "%d")
TOLLVM_VEC_TOSTRING_IMPL(int32_t,  i32,  "%d")
TOLLVM_VEC_TOSTRING_IMPL(int64_t,  i64,  "%lld")
TOLLVM_VEC_TOSTRING_IMPL(uint8_t,  u8,   "%u")
TOLLVM_VEC_TOSTRING_IMPL(uint16_t, u16,  "%u")
TOLLVM_VEC_TOSTRING_IMPL(uint32_t, u32,  "%u")
TOLLVM_VEC_TOSTRING_IMPL(uint64_t, u64,  "%llu")
TOLLVM_VEC_TOSTRING_IMPL(float,    f32,  "%g")
TOLLVM_VEC_TOSTRING_IMPL(double,   f64,  "%g")
TOLLVM_VEC_TOSTRING_IMPL(char,     char, "%c")

// ═══════════════════════════════════════════════════════════════════════════════
// Vec<string> — manual implementation (struct comparison needs memcmp)
// ═══════════════════════════════════════════════════════════════════════════════

static inline tollvm_string* vec_data_str(tollvm_vec_header* v) {
    return (tollvm_string*)v->ptr;
}
static inline const tollvm_string* vec_cdata_str(const tollvm_vec_header* v) {
    return (const tollvm_string*)v->ptr;
}

static void vec_grow_str(tollvm_vec_header* v, size_t needed) {
    if (needed <= v->cap) return;
    size_t newCap = v->cap ? v->cap : TOLLVM_VEC_INITIAL_CAP;
    while (newCap < needed) newCap *= TOLLVM_VEC_GROWTH_FACTOR;
    v->ptr = realloc(v->ptr, newCap * sizeof(tollvm_string));
    v->cap = newCap;
}

static int str_equal(tollvm_string a, tollvm_string b) {
    return a.len == b.len && (a.ptr == b.ptr || memcmp(a.ptr, b.ptr, a.len) == 0);
}

// ── Creation / destruction ──────────────────────────────────────────────────
void tollvm_vec_init_str(tollvm_vec_header* v) {
    v->ptr = NULL; v->len = 0; v->cap = 0;
}
void tollvm_vec_init_cap_str(tollvm_vec_header* v, size_t cap) {
    v->ptr = malloc(cap * sizeof(tollvm_string)); v->len = 0; v->cap = cap;
}
void tollvm_vec_free_str(tollvm_vec_header* v) {
    free(v->ptr); v->ptr = NULL; v->len = 0; v->cap = 0;
}

// ── Size / capacity ─────────────────────────────────────────────────────────
size_t tollvm_vec_len_str(const tollvm_vec_header* v) { return v->len; }
size_t tollvm_vec_capacity_str(const tollvm_vec_header* v) { return v->cap; }
int    tollvm_vec_isEmpty_str(const tollvm_vec_header* v) { return v->len == 0; }

// ── Element access ──────────────────────────────────────────────────────────
tollvm_string tollvm_vec_at_str(const tollvm_vec_header* v, size_t idx) {
    if (idx >= v->len) {
        fprintf(stderr, "tollvm: vec index out of bounds: %zu >= %zu\n", idx, v->len);
        exit(1);
    }
    return vec_cdata_str(v)[idx];
}
tollvm_string tollvm_vec_first_str(const tollvm_vec_header* v) {
    if (v->len == 0) { fprintf(stderr, "tollvm: vec.first() on empty vec\n"); exit(1); }
    return vec_cdata_str(v)[0];
}
tollvm_string tollvm_vec_last_str(const tollvm_vec_header* v) {
    if (v->len == 0) { fprintf(stderr, "tollvm: vec.last() on empty vec\n"); exit(1); }
    return vec_cdata_str(v)[v->len - 1];
}
void tollvm_vec_set_str(tollvm_vec_header* v, size_t idx, tollvm_string val) {
    if (idx >= v->len) {
        fprintf(stderr, "tollvm: vec index out of bounds: %zu >= %zu\n", idx, v->len);
        exit(1);
    }
    vec_data_str(v)[idx] = val;
}

// ── Mutation ────────────────────────────────────────────────────────────────
void tollvm_vec_push_str(tollvm_vec_header* v, tollvm_string val) {
    vec_grow_str(v, v->len + 1);
    vec_data_str(v)[v->len++] = val;
}
tollvm_string tollvm_vec_pop_str(tollvm_vec_header* v) {
    if (v->len == 0) { fprintf(stderr, "tollvm: vec.pop() on empty vec\n"); exit(1); }
    return vec_data_str(v)[--v->len];
}
void tollvm_vec_insert_str(tollvm_vec_header* v, size_t idx, tollvm_string val) {
    if (idx > v->len) {
        fprintf(stderr, "tollvm: vec.insert() index out of bounds\n"); exit(1);
    }
    vec_grow_str(v, v->len + 1);
    tollvm_string* d = vec_data_str(v);
    memmove(&d[idx + 1], &d[idx], (v->len - idx) * sizeof(tollvm_string));
    d[idx] = val;
    v->len++;
}
tollvm_string tollvm_vec_removeAt_str(tollvm_vec_header* v, size_t idx) {
    if (idx >= v->len) {
        fprintf(stderr, "tollvm: vec.removeAt() index out of bounds\n"); exit(1);
    }
    tollvm_string* d = vec_data_str(v);
    tollvm_string val = d[idx];
    memmove(&d[idx], &d[idx + 1], (v->len - idx - 1) * sizeof(tollvm_string));
    v->len--;
    return val;
}
tollvm_string tollvm_vec_removeSwap_str(tollvm_vec_header* v, size_t idx) {
    if (idx >= v->len) {
        fprintf(stderr, "tollvm: vec.removeSwap() index out of bounds\n"); exit(1);
    }
    tollvm_string* d = vec_data_str(v);
    tollvm_string val = d[idx];
    d[idx] = d[v->len - 1];
    v->len--;
    return val;
}
void tollvm_vec_clear_str(tollvm_vec_header* v) { v->len = 0; }
void tollvm_vec_fill_str(tollvm_vec_header* v, tollvm_string val) {
    tollvm_string* d = vec_data_str(v);
    for (size_t i = 0; i < v->len; i++) d[i] = val;
}
void tollvm_vec_swap_str(tollvm_vec_header* v, size_t i, size_t j) {
    if (i >= v->len || j >= v->len) {
        fprintf(stderr, "tollvm: vec.swap() index out of bounds\n"); exit(1);
    }
    tollvm_string* d = vec_data_str(v);
    tollvm_string tmp = d[i]; d[i] = d[j]; d[j] = tmp;
}

// ── Memory ──────────────────────────────────────────────────────────────────
void tollvm_vec_reserve_str(tollvm_vec_header* v, size_t cap) {
    vec_grow_str(v, cap);
}
void tollvm_vec_shrink_str(tollvm_vec_header* v) {
    if (v->len == 0) { free(v->ptr); v->ptr = NULL; v->cap = 0; }
    else if (v->cap > v->len) {
        v->ptr = realloc(v->ptr, v->len * sizeof(tollvm_string));
        v->cap = v->len;
    }
}
void tollvm_vec_resize_str(tollvm_vec_header* v, size_t len, tollvm_string fill) {
    vec_grow_str(v, len);
    tollvm_string* d = vec_data_str(v);
    for (size_t i = v->len; i < len; i++) d[i] = fill;
    v->len = len;
}
void tollvm_vec_truncate_str(tollvm_vec_header* v, size_t len) {
    if (len < v->len) v->len = len;
}

// ── Search (string comparison via memcmp) ───────────────────────────────────
int tollvm_vec_contains_str(const tollvm_vec_header* v, tollvm_string val) {
    const tollvm_string* d = vec_cdata_str(v);
    for (size_t i = 0; i < v->len; i++)
        if (str_equal(d[i], val)) return 1;
    return 0;
}
long long tollvm_vec_indexOf_str(const tollvm_vec_header* v, tollvm_string val) {
    const tollvm_string* d = vec_cdata_str(v);
    for (size_t i = 0; i < v->len; i++)
        if (str_equal(d[i], val)) return (long long)i;
    return -1;
}
long long tollvm_vec_lastIndexOf_str(const tollvm_vec_header* v, tollvm_string val) {
    const tollvm_string* d = vec_cdata_str(v);
    for (size_t i = v->len; i > 0; i--)
        if (str_equal(d[i - 1], val)) return (long long)(i - 1);
    return -1;
}
size_t tollvm_vec_count_str(const tollvm_vec_header* v, tollvm_string val) {
    const tollvm_string* d = vec_cdata_str(v);
    size_t c = 0;
    for (size_t i = 0; i < v->len; i++)
        if (str_equal(d[i], val)) c++;
    return c;
}

// ── Reorder ─────────────────────────────────────────────────────────────────
void tollvm_vec_reverse_str(tollvm_vec_header* v) {
    tollvm_string* d = vec_data_str(v);
    for (size_t i = 0, j = v->len - 1; i < j; i++, j--) {
        tollvm_string tmp = d[i]; d[i] = d[j]; d[j] = tmp;
    }
}

// ── Comparison ──────────────────────────────────────────────────────────────
int tollvm_vec_equals_str(const tollvm_vec_header* a, const tollvm_vec_header* b) {
    if (a->len != b->len) return 0;
    const tollvm_string* da = vec_cdata_str(a);
    const tollvm_string* db = (const tollvm_string*)b->ptr;
    for (size_t i = 0; i < a->len; i++)
        if (!str_equal(da[i], db[i])) return 0;
    return 1;
}

// ── Rotate ──────────────────────────────────────────────────────────────────
void tollvm_vec_rotate_str(tollvm_vec_header* v, int32_t steps) {
    if (v->len <= 1) return;
    int64_t n = (int64_t)v->len;
    int64_t s = ((int64_t)steps % n + n) % n;
    if (s == 0) return;
    tollvm_string* d = vec_data_str(v);
    for (size_t i = 0, j = v->len - 1; i < j; i++, j--) {
        tollvm_string tmp = d[i]; d[i] = d[j]; d[j] = tmp; }
    for (size_t i = 0, j = (size_t)s - 1; i < j; i++, j--) {
        tollvm_string tmp = d[i]; d[i] = d[j]; d[j] = tmp; }
    for (size_t i = (size_t)s, j = v->len - 1; i < j; i++, j--) {
        tollvm_string tmp = d[i]; d[i] = d[j]; d[j] = tmp; }
}

// ── Conversion ──────────────────────────────────────────────────────────────
tollvm_string tollvm_vec_toString_str(const tollvm_vec_header* v) {
    if (v->len == 0) {
        tollvm_string r = { "[]", 2 }; return r;
    }
    size_t cap = 64;
    char* buf = (char*)malloc(cap);
    size_t pos = 0;
    buf[pos++] = '[';
    const tollvm_string* d = vec_cdata_str(v);
    for (size_t i = 0; i < v->len; i++) {
        if (i > 0) { buf[pos++] = ','; buf[pos++] = ' '; }
        size_t need = d[i].len + 3; // quotes + content
        while (cap - pos < need) { cap *= 2; buf = (char*)realloc(buf, cap); }
        buf[pos++] = '"';
        memcpy(buf + pos, d[i].ptr, d[i].len); pos += d[i].len;
        buf[pos++] = '"';
    }
    if (pos + 1 >= cap) { cap = pos + 2; buf = (char*)realloc(buf, cap); }
    buf[pos++] = ']';
    buf[pos] = '\0';
    tollvm_string r = { buf, pos }; return r;
}

tollvm_string tollvm_vec_join_str(const tollvm_vec_header* v, tollvm_string sep) {
    if (v->len == 0) { tollvm_string r = { "", 0 }; return r; }
    const tollvm_string* d = vec_cdata_str(v);
    // Calculate total size
    size_t total = 0;
    for (size_t i = 0; i < v->len; i++) {
        if (i > 0) total += sep.len;
        total += d[i].len;
    }
    char* buf = (char*)malloc(total + 1);
    size_t pos = 0;
    for (size_t i = 0; i < v->len; i++) {
        if (i > 0) { memcpy(buf + pos, sep.ptr, sep.len); pos += sep.len; }
        memcpy(buf + pos, d[i].ptr, d[i].len); pos += d[i].len;
    }
    buf[pos] = '\0';
    tollvm_string r = { buf, pos }; return r;
}

// ── Clone ───────────────────────────────────────────────────────────────────
void tollvm_vec_clone_str(const tollvm_vec_header* src, tollvm_vec_header* dst) {
    dst->len = src->len;
    dst->cap = src->len;
    if (src->len > 0) {
        dst->ptr = malloc(src->len * sizeof(tollvm_string));
        memcpy(dst->ptr, src->ptr, src->len * sizeof(tollvm_string));
    } else {
        dst->ptr = NULL;
        dst->cap = 0;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Args helper — converts C main(argc, argv) → Vec<string>
// ═══════════════════════════════════════════════════════════════════════════════

void tollvm_args_init(tollvm_vec_header* out, int argc, const char** argv) {
    tollvm_vec_init_cap_str(out, (size_t)argc);
    for (int i = 0; i < argc; i++) {
        tollvm_string s;
        s.ptr = argv[i];
        s.len = strlen(argv[i]);
        tollvm_vec_push_str(out, s);
    }
}
