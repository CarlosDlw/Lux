#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// Vec<T> runtime — monomorphized per element type
//
// Naming convention: tollvm_vec_<method>_<suffix>(vec_ptr, ...)
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
} tollvm_vec_header;

// ── Macro to declare all vec functions for a given element type ──────────────
#define TOLLVM_VEC_DECLARE(T, SUFFIX)                                           \
    /* Creation / destruction */                                                \
    void tollvm_vec_init_##SUFFIX(tollvm_vec_header* v);                        \
    void tollvm_vec_init_cap_##SUFFIX(tollvm_vec_header* v, size_t cap);        \
    void tollvm_vec_free_##SUFFIX(tollvm_vec_header* v);                        \
    /* Size / capacity */                                                       \
    size_t tollvm_vec_len_##SUFFIX(const tollvm_vec_header* v);                 \
    size_t tollvm_vec_capacity_##SUFFIX(const tollvm_vec_header* v);            \
    int    tollvm_vec_isEmpty_##SUFFIX(const tollvm_vec_header* v);             \
    /* Element access */                                                        \
    T    tollvm_vec_at_##SUFFIX(const tollvm_vec_header* v, size_t idx);        \
    T    tollvm_vec_first_##SUFFIX(const tollvm_vec_header* v);                 \
    T    tollvm_vec_last_##SUFFIX(const tollvm_vec_header* v);                  \
    void tollvm_vec_set_##SUFFIX(tollvm_vec_header* v, size_t idx, T val);     \
    /* Mutation */                                                              \
    void tollvm_vec_push_##SUFFIX(tollvm_vec_header* v, T val);                 \
    T    tollvm_vec_pop_##SUFFIX(tollvm_vec_header* v);                         \
    void tollvm_vec_insert_##SUFFIX(tollvm_vec_header* v, size_t idx, T val);   \
    T    tollvm_vec_removeAt_##SUFFIX(tollvm_vec_header* v, size_t idx);        \
    T    tollvm_vec_removeSwap_##SUFFIX(tollvm_vec_header* v, size_t idx);      \
    void tollvm_vec_clear_##SUFFIX(tollvm_vec_header* v);                       \
    void tollvm_vec_fill_##SUFFIX(tollvm_vec_header* v, T val);                 \
    void tollvm_vec_swap_##SUFFIX(tollvm_vec_header* v, size_t i, size_t j);    \
    /* Memory */                                                                \
    void tollvm_vec_reserve_##SUFFIX(tollvm_vec_header* v, size_t cap);         \
    void tollvm_vec_shrink_##SUFFIX(tollvm_vec_header* v);                      \
    void tollvm_vec_resize_##SUFFIX(tollvm_vec_header* v, size_t len, T fill);  \
    void tollvm_vec_truncate_##SUFFIX(tollvm_vec_header* v, size_t len);        \
    /* Search */                                                                \
    int    tollvm_vec_contains_##SUFFIX(const tollvm_vec_header* v, T val);     \
    long long tollvm_vec_indexOf_##SUFFIX(const tollvm_vec_header* v, T val);   \
    long long tollvm_vec_lastIndexOf_##SUFFIX(const tollvm_vec_header* v, T val);\
    size_t tollvm_vec_count_##SUFFIX(const tollvm_vec_header* v, T val);        \
    /* Comparison */                                                            \
    int    tollvm_vec_equals_##SUFFIX(const tollvm_vec_header* a,               \
                                      const tollvm_vec_header* b);              \
    int    tollvm_vec_isSorted_##SUFFIX(const tollvm_vec_header* v);            \
    /* Reorder */                                                               \
    void tollvm_vec_reverse_##SUFFIX(tollvm_vec_header* v);                     \
    void tollvm_vec_sort_##SUFFIX(tollvm_vec_header* v);                        \
    void tollvm_vec_sortDesc_##SUFFIX(tollvm_vec_header* v);                    \
    void tollvm_vec_rotate_##SUFFIX(tollvm_vec_header* v, int32_t steps);       \
    /* Aggregation */                                                           \
    T    tollvm_vec_sum_##SUFFIX(const tollvm_vec_header* v);                   \
    T    tollvm_vec_product_##SUFFIX(const tollvm_vec_header* v);               \
    T    tollvm_vec_min_##SUFFIX(const tollvm_vec_header* v);                   \
    T    tollvm_vec_max_##SUFFIX(const tollvm_vec_header* v);                   \
    double tollvm_vec_average_##SUFFIX(const tollvm_vec_header* v);             \
    /* Clone */                                                                 \
    void tollvm_vec_clone_##SUFFIX(const tollvm_vec_header* src, tollvm_vec_header* dst);

// Declare for all primitive types
TOLLVM_VEC_DECLARE(int8_t,               i8)
TOLLVM_VEC_DECLARE(int16_t,              i16)
TOLLVM_VEC_DECLARE(int32_t,              i32)
TOLLVM_VEC_DECLARE(int64_t,              i64)
TOLLVM_VEC_DECLARE(uint8_t,              u8)
TOLLVM_VEC_DECLARE(uint16_t,             u16)
TOLLVM_VEC_DECLARE(uint32_t,             u32)
TOLLVM_VEC_DECLARE(uint64_t,             u64)
TOLLVM_VEC_DECLARE(float,                f32)
TOLLVM_VEC_DECLARE(double,               f64)
TOLLVM_VEC_DECLARE(char,                 char)

// ── String type for Vec<string> ─────────────────────────────────────────────
// String is a fat pointer { ptr, len }, not a simple scalar.
// Search functions (contains, indexOf, etc.) need custom comparison,
// so we declare them manually instead of using the macro.

typedef struct {
    const char* ptr;
    size_t      len;
} tollvm_string;

// ── toString / join — declared per type (need tollvm_string defined above) ──
#define TOLLVM_VEC_DECLARE_TOSTRING(SUFFIX)                                     \
    tollvm_string tollvm_vec_toString_##SUFFIX(const tollvm_vec_header* v);      \
    tollvm_string tollvm_vec_join_##SUFFIX(const tollvm_vec_header* v,           \
                                           tollvm_string sep);

TOLLVM_VEC_DECLARE_TOSTRING(i8)
TOLLVM_VEC_DECLARE_TOSTRING(i16)
TOLLVM_VEC_DECLARE_TOSTRING(i32)
TOLLVM_VEC_DECLARE_TOSTRING(i64)
TOLLVM_VEC_DECLARE_TOSTRING(u8)
TOLLVM_VEC_DECLARE_TOSTRING(u16)
TOLLVM_VEC_DECLARE_TOSTRING(u32)
TOLLVM_VEC_DECLARE_TOSTRING(u64)
TOLLVM_VEC_DECLARE_TOSTRING(f32)
TOLLVM_VEC_DECLARE_TOSTRING(f64)
TOLLVM_VEC_DECLARE_TOSTRING(char)

// Creation / destruction
void   tollvm_vec_init_str(tollvm_vec_header* v);
void   tollvm_vec_init_cap_str(tollvm_vec_header* v, size_t cap);
void   tollvm_vec_free_str(tollvm_vec_header* v);

// Size / capacity
size_t tollvm_vec_len_str(const tollvm_vec_header* v);
size_t tollvm_vec_capacity_str(const tollvm_vec_header* v);
int    tollvm_vec_isEmpty_str(const tollvm_vec_header* v);

// Element access
tollvm_string tollvm_vec_at_str(const tollvm_vec_header* v, size_t idx);
tollvm_string tollvm_vec_first_str(const tollvm_vec_header* v);
tollvm_string tollvm_vec_last_str(const tollvm_vec_header* v);
void          tollvm_vec_set_str(tollvm_vec_header* v, size_t idx, tollvm_string val);

// Mutation
void          tollvm_vec_push_str(tollvm_vec_header* v, tollvm_string val);
tollvm_string tollvm_vec_pop_str(tollvm_vec_header* v);
void          tollvm_vec_insert_str(tollvm_vec_header* v, size_t idx, tollvm_string val);
tollvm_string tollvm_vec_removeAt_str(tollvm_vec_header* v, size_t idx);
tollvm_string tollvm_vec_removeSwap_str(tollvm_vec_header* v, size_t idx);
void          tollvm_vec_clear_str(tollvm_vec_header* v);
void          tollvm_vec_fill_str(tollvm_vec_header* v, tollvm_string val);
void          tollvm_vec_swap_str(tollvm_vec_header* v, size_t i, size_t j);

// Memory
void   tollvm_vec_reserve_str(tollvm_vec_header* v, size_t cap);
void   tollvm_vec_shrink_str(tollvm_vec_header* v);
void   tollvm_vec_resize_str(tollvm_vec_header* v, size_t len, tollvm_string fill);
void   tollvm_vec_truncate_str(tollvm_vec_header* v, size_t len);

// Search (uses memcmp for string comparison)
int       tollvm_vec_contains_str(const tollvm_vec_header* v, tollvm_string val);
long long tollvm_vec_indexOf_str(const tollvm_vec_header* v, tollvm_string val);
long long tollvm_vec_lastIndexOf_str(const tollvm_vec_header* v, tollvm_string val);
size_t    tollvm_vec_count_str(const tollvm_vec_header* v, tollvm_string val);

// Comparison
int       tollvm_vec_equals_str(const tollvm_vec_header* a, const tollvm_vec_header* b);

// Reorder
void tollvm_vec_reverse_str(tollvm_vec_header* v);
void tollvm_vec_rotate_str(tollvm_vec_header* v, int32_t steps);

// Conversion
tollvm_string tollvm_vec_toString_str(const tollvm_vec_header* v);
tollvm_string tollvm_vec_join_str(const tollvm_vec_header* v, tollvm_string sep);

// Clone
void tollvm_vec_clone_str(const tollvm_vec_header* src, tollvm_vec_header* dst);

// ── Args helper ─────────────────────────────────────────────────────────────
// Converts C main(argc, argv) into a Vec<string> for tollvm programs.
void tollvm_args_init(tollvm_vec_header* out, int argc, const char** argv);

#ifdef __cplusplus
}
#endif
