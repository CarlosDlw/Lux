#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// Vec<T> runtime — monomorphized per element type
//
// Naming convention: lux_vec_<method>_<suffix>(vec_ptr, ...)
//
// Vec layout (same for all element types):
//   struct { void* ptr; size_t len; size_t cap; }
//
// The vec struct is always passed and manipulated by pointer.
// ═══════════════════════════════════════════════════════════════════════════════

// Generic vec struct layout (element-type-agnostic header)
typedef struct {
    void*  ptr;
    size_t len;
    size_t cap;
} lux_vec_header;

// ── Macro to declare all vec functions for a given element type ──────────────
#define LUX_VEC_DECLARE(T, SUFFIX)                                           \
    /* Creation / destruction */                                                \
    void lux_vec_init_##SUFFIX(lux_vec_header* v);                        \
    void lux_vec_init_cap_##SUFFIX(lux_vec_header* v, size_t cap);        \
    void lux_vec_free_##SUFFIX(lux_vec_header* v);                        \
    /* Size / capacity */                                                       \
    size_t lux_vec_len_##SUFFIX(const lux_vec_header* v);                 \
    size_t lux_vec_capacity_##SUFFIX(const lux_vec_header* v);            \
    int    lux_vec_isEmpty_##SUFFIX(const lux_vec_header* v);             \
    /* Element access */                                                        \
    T    lux_vec_at_##SUFFIX(const lux_vec_header* v, size_t idx);        \
    T    lux_vec_first_##SUFFIX(const lux_vec_header* v);                 \
    T    lux_vec_last_##SUFFIX(const lux_vec_header* v);                  \
    void lux_vec_set_##SUFFIX(lux_vec_header* v, size_t idx, T val);     \
    /* Mutation */                                                              \
    void lux_vec_push_##SUFFIX(lux_vec_header* v, T val);                 \
    T    lux_vec_pop_##SUFFIX(lux_vec_header* v);                         \
    void lux_vec_insert_##SUFFIX(lux_vec_header* v, size_t idx, T val);   \
    T    lux_vec_removeAt_##SUFFIX(lux_vec_header* v, size_t idx);        \
    T    lux_vec_removeSwap_##SUFFIX(lux_vec_header* v, size_t idx);      \
    void lux_vec_clear_##SUFFIX(lux_vec_header* v);                       \
    void lux_vec_fill_##SUFFIX(lux_vec_header* v, T val);                 \
    void lux_vec_swap_##SUFFIX(lux_vec_header* v, size_t i, size_t j);    \
    /* Memory */                                                                \
    void lux_vec_reserve_##SUFFIX(lux_vec_header* v, size_t cap);         \
    void lux_vec_shrink_##SUFFIX(lux_vec_header* v);                      \
    void lux_vec_resize_##SUFFIX(lux_vec_header* v, size_t len, T fill);  \
    void lux_vec_truncate_##SUFFIX(lux_vec_header* v, size_t len);        \
    /* Search */                                                                \
    int    lux_vec_contains_##SUFFIX(const lux_vec_header* v, T val);     \
    long long lux_vec_indexOf_##SUFFIX(const lux_vec_header* v, T val);   \
    long long lux_vec_lastIndexOf_##SUFFIX(const lux_vec_header* v, T val);\
    size_t lux_vec_count_##SUFFIX(const lux_vec_header* v, T val);        \
    /* Comparison */                                                            \
    int    lux_vec_equals_##SUFFIX(const lux_vec_header* a,               \
                                      const lux_vec_header* b);              \
    int    lux_vec_isSorted_##SUFFIX(const lux_vec_header* v);            \
    /* Reorder */                                                               \
    void lux_vec_reverse_##SUFFIX(lux_vec_header* v);                     \
    void lux_vec_sort_##SUFFIX(lux_vec_header* v);                        \
    void lux_vec_sortDesc_##SUFFIX(lux_vec_header* v);                    \
    void lux_vec_rotate_##SUFFIX(lux_vec_header* v, int32_t steps);       \
    /* Aggregation */                                                           \
    T    lux_vec_sum_##SUFFIX(const lux_vec_header* v);                   \
    T    lux_vec_product_##SUFFIX(const lux_vec_header* v);               \
    T    lux_vec_min_##SUFFIX(const lux_vec_header* v);                   \
    T    lux_vec_max_##SUFFIX(const lux_vec_header* v);                   \
    double lux_vec_average_##SUFFIX(const lux_vec_header* v);             \
    /* Clone */                                                                 \
    void lux_vec_clone_##SUFFIX(const lux_vec_header* src, lux_vec_header* dst);

// Declare for all primitive types
LUX_VEC_DECLARE(int8_t,               i8)
LUX_VEC_DECLARE(int16_t,              i16)
LUX_VEC_DECLARE(int32_t,              i32)
LUX_VEC_DECLARE(int64_t,              i64)
LUX_VEC_DECLARE(uint8_t,              u8)
LUX_VEC_DECLARE(uint16_t,             u16)
LUX_VEC_DECLARE(uint32_t,             u32)
LUX_VEC_DECLARE(uint64_t,             u64)
LUX_VEC_DECLARE(float,                f32)
LUX_VEC_DECLARE(double,               f64)
LUX_VEC_DECLARE(char,                 char)

// ── String type for Vec<string> ─────────────────────────────────────────────
// String is a fat pointer { ptr, len }, not a simple scalar.
// Search functions (contains, indexOf, etc.) need custom comparison,
// so we declare them manually instead of using the macro.

typedef struct {
    const char* ptr;
    size_t      len;
} lux_string;

// ── toString / join — declared per type (need lux_string defined above) ──
#define LUX_VEC_DECLARE_TOSTRING(SUFFIX)                                     \
    lux_string lux_vec_toString_##SUFFIX(const lux_vec_header* v);      \
    lux_string lux_vec_join_##SUFFIX(const lux_vec_header* v,           \
                                           lux_string sep);

LUX_VEC_DECLARE_TOSTRING(i8)
LUX_VEC_DECLARE_TOSTRING(i16)
LUX_VEC_DECLARE_TOSTRING(i32)
LUX_VEC_DECLARE_TOSTRING(i64)
LUX_VEC_DECLARE_TOSTRING(u8)
LUX_VEC_DECLARE_TOSTRING(u16)
LUX_VEC_DECLARE_TOSTRING(u32)
LUX_VEC_DECLARE_TOSTRING(u64)
LUX_VEC_DECLARE_TOSTRING(f32)
LUX_VEC_DECLARE_TOSTRING(f64)
LUX_VEC_DECLARE_TOSTRING(char)

// Creation / destruction
void   lux_vec_init_str(lux_vec_header* v);
void   lux_vec_init_cap_str(lux_vec_header* v, size_t cap);
void   lux_vec_free_str(lux_vec_header* v);

// Size / capacity
size_t lux_vec_len_str(const lux_vec_header* v);
size_t lux_vec_capacity_str(const lux_vec_header* v);
int    lux_vec_isEmpty_str(const lux_vec_header* v);

// Element access
lux_string lux_vec_at_str(const lux_vec_header* v, size_t idx);
lux_string lux_vec_first_str(const lux_vec_header* v);
lux_string lux_vec_last_str(const lux_vec_header* v);
void          lux_vec_set_str(lux_vec_header* v, size_t idx, lux_string val);

// Mutation
void          lux_vec_push_str(lux_vec_header* v, lux_string val);
lux_string lux_vec_pop_str(lux_vec_header* v);
void          lux_vec_insert_str(lux_vec_header* v, size_t idx, lux_string val);
lux_string lux_vec_removeAt_str(lux_vec_header* v, size_t idx);
lux_string lux_vec_removeSwap_str(lux_vec_header* v, size_t idx);
void          lux_vec_clear_str(lux_vec_header* v);
void          lux_vec_fill_str(lux_vec_header* v, lux_string val);
void          lux_vec_swap_str(lux_vec_header* v, size_t i, size_t j);

// Memory
void   lux_vec_reserve_str(lux_vec_header* v, size_t cap);
void   lux_vec_shrink_str(lux_vec_header* v);
void   lux_vec_resize_str(lux_vec_header* v, size_t len, lux_string fill);
void   lux_vec_truncate_str(lux_vec_header* v, size_t len);

// Search (uses memcmp for string comparison)
int       lux_vec_contains_str(const lux_vec_header* v, lux_string val);
long long lux_vec_indexOf_str(const lux_vec_header* v, lux_string val);
long long lux_vec_lastIndexOf_str(const lux_vec_header* v, lux_string val);
size_t    lux_vec_count_str(const lux_vec_header* v, lux_string val);

// Comparison
int       lux_vec_equals_str(const lux_vec_header* a, const lux_vec_header* b);

// Reorder
void lux_vec_reverse_str(lux_vec_header* v);
void lux_vec_rotate_str(lux_vec_header* v, int32_t steps);

// Conversion
lux_string lux_vec_toString_str(const lux_vec_header* v);
lux_string lux_vec_join_str(const lux_vec_header* v, lux_string sep);

// Clone
void lux_vec_clone_str(const lux_vec_header* src, lux_vec_header* dst);

// ── Args helper ─────────────────────────────────────────────────────────────
// Converts C main(argc, argv) into a Vec<string> for lux programs.
void lux_args_init(lux_vec_header* out, int argc, const char** argv);

#ifdef __cplusplus
}
#endif
