#ifndef LUX_MAP_H
#define LUX_MAP_H

#include <stddef.h>
#include <stdint.h>

// ═══════════════════════════════════════════════════════════════════════════
// Map<K,V> — Open-addressing hash map
//
// LLVM struct: { ptr states, ptr keys, ptr values, ptr hashes,
//                usize len, usize cap, usize key_size, usize val_size }
// C struct mirrors LLVM layout exactly.
// ═══════════════════════════════════════════════════════════════════════════

typedef struct {
    const char* ptr;
    size_t      len;
} lux_map_string;

typedef struct {
    uint8_t*  states;     // 0=empty, 1=occupied, 2=tombstone
    void*     keys;       // flat key array   (key_size * cap bytes)
    void*     values;     // flat value array  (val_size * cap bytes)
    uint64_t* hashes;     // cached hashes per slot
    size_t    len;        // active entries
    size_t    cap;        // total slots
    size_t    key_size;   // bytes per key
    size_t    val_size;   // bytes per value
} lux_map_header;

// Vec out-parameter (mirrors lux_vec_header layout)
typedef struct {
    void*  ptr;
    size_t len;
    size_t cap;
} lux_map_vec_out;

// ── Macro-generated declarations ─────────────────────────────────────────

// For integer keys: KT = C type, KS = suffix
#define LUX_MAP_DECL_INT_KEY(KT, KS, VT, VS)                             \
void   lux_map_init_##KS##_##VS(lux_map_header* m);                    \
void   lux_map_free_##KS##_##VS(lux_map_header* m);                    \
size_t lux_map_len_##KS##_##VS(const lux_map_header* m);               \
int    lux_map_isEmpty_##KS##_##VS(const lux_map_header* m);           \
void   lux_map_set_##KS##_##VS(lux_map_header* m, KT key, VT val);    \
VT     lux_map_get_##KS##_##VS(lux_map_header* m, KT key);            \
VT     lux_map_getOrDefault_##KS##_##VS(lux_map_header* m,             \
                                           KT key, VT def);                 \
int    lux_map_has_##KS##_##VS(lux_map_header* m, KT key);            \
int    lux_map_remove_##KS##_##VS(lux_map_header* m, KT key);         \
void   lux_map_clear_##KS##_##VS(lux_map_header* m);                  \
void   lux_map_keys_##KS##_##VS(lux_map_header* m,                     \
                                    lux_map_vec_out* out);               \
void   lux_map_values_##KS##_##VS(lux_map_header* m,                   \
                                      lux_map_vec_out* out);

// For string keys: key is always lux_map_string
#define LUX_MAP_DECL_STR_KEY(VT, VS)                                      \
void   lux_map_init_str_##VS(lux_map_header* m);                       \
void   lux_map_free_str_##VS(lux_map_header* m);                       \
size_t lux_map_len_str_##VS(const lux_map_header* m);                  \
int    lux_map_isEmpty_str_##VS(const lux_map_header* m);              \
void   lux_map_set_str_##VS(lux_map_header* m,                         \
                               lux_map_string key, VT val);              \
VT     lux_map_get_str_##VS(lux_map_header* m,                         \
                               lux_map_string key);                      \
VT     lux_map_getOrDefault_str_##VS(lux_map_header* m,                \
                               lux_map_string key, VT def);              \
int    lux_map_has_str_##VS(lux_map_header* m,                         \
                               lux_map_string key);                      \
int    lux_map_remove_str_##VS(lux_map_header* m,                      \
                               lux_map_string key);                      \
void   lux_map_clear_str_##VS(lux_map_header* m);                      \
void   lux_map_keys_str_##VS(lux_map_header* m,                        \
                                 lux_map_vec_out* out);                  \
void   lux_map_values_str_##VS(lux_map_header* m,                      \
                                   lux_map_vec_out* out);

// String key + string value (special return type)
#define LUX_MAP_DECL_STR_STR()                                            \
void             lux_map_init_str_str(lux_map_header* m);              \
void             lux_map_free_str_str(lux_map_header* m);              \
size_t           lux_map_len_str_str(const lux_map_header* m);         \
int              lux_map_isEmpty_str_str(const lux_map_header* m);     \
void             lux_map_set_str_str(lux_map_header* m,                \
                                        lux_map_string key,              \
                                        lux_map_string val);             \
lux_map_string lux_map_get_str_str(lux_map_header* m,               \
                                        lux_map_string key);             \
lux_map_string lux_map_getOrDefault_str_str(lux_map_header* m,      \
                                        lux_map_string key,              \
                                        lux_map_string def);             \
int              lux_map_has_str_str(lux_map_header* m,                \
                                        lux_map_string key);             \
int              lux_map_remove_str_str(lux_map_header* m,             \
                                        lux_map_string key);             \
void             lux_map_clear_str_str(lux_map_header* m);             \
void             lux_map_keys_str_str(lux_map_header* m,               \
                                          lux_map_vec_out* out);         \
void             lux_map_values_str_str(lux_map_header* m,             \
                                            lux_map_vec_out* out);

// Integer key + string value
#define LUX_MAP_DECL_INT_KEY_STR_VAL(KT, KS)                             \
void             lux_map_init_##KS##_str(lux_map_header* m);           \
void             lux_map_free_##KS##_str(lux_map_header* m);           \
size_t           lux_map_len_##KS##_str(const lux_map_header* m);      \
int              lux_map_isEmpty_##KS##_str(const lux_map_header* m);  \
void             lux_map_set_##KS##_str(lux_map_header* m,             \
                                           KT key, lux_map_string val);  \
lux_map_string lux_map_get_##KS##_str(lux_map_header* m, KT key);   \
lux_map_string lux_map_getOrDefault_##KS##_str(lux_map_header* m,   \
                                           KT key, lux_map_string def);  \
int              lux_map_has_##KS##_str(lux_map_header* m, KT key);    \
int              lux_map_remove_##KS##_str(lux_map_header* m, KT key); \
void             lux_map_clear_##KS##_str(lux_map_header* m);          \
void             lux_map_keys_##KS##_str(lux_map_header* m,            \
                                             lux_map_vec_out* out);      \
void             lux_map_values_##KS##_str(lux_map_header* m,          \
                                               lux_map_vec_out* out);

// ── Instantiate declarations ─────────────────────────────────────────────

// String key × numeric values
LUX_MAP_DECL_STR_KEY(int8_t,   i8)
LUX_MAP_DECL_STR_KEY(int16_t,  i16)
LUX_MAP_DECL_STR_KEY(int32_t,  i32)
LUX_MAP_DECL_STR_KEY(int64_t,  i64)
LUX_MAP_DECL_STR_KEY(uint8_t,  u8)
LUX_MAP_DECL_STR_KEY(uint16_t, u16)
LUX_MAP_DECL_STR_KEY(uint32_t, u32)
LUX_MAP_DECL_STR_KEY(uint64_t, u64)
LUX_MAP_DECL_STR_KEY(float,    f32)
LUX_MAP_DECL_STR_KEY(double,   f64)

// String key × string value
LUX_MAP_DECL_STR_STR()

// Int32 key × all values
LUX_MAP_DECL_INT_KEY(int32_t, i32, int8_t,   i8)
LUX_MAP_DECL_INT_KEY(int32_t, i32, int16_t,  i16)
LUX_MAP_DECL_INT_KEY(int32_t, i32, int32_t,  i32)
LUX_MAP_DECL_INT_KEY(int32_t, i32, int64_t,  i64)
LUX_MAP_DECL_INT_KEY(int32_t, i32, uint8_t,  u8)
LUX_MAP_DECL_INT_KEY(int32_t, i32, uint16_t, u16)
LUX_MAP_DECL_INT_KEY(int32_t, i32, uint32_t, u32)
LUX_MAP_DECL_INT_KEY(int32_t, i32, uint64_t, u64)
LUX_MAP_DECL_INT_KEY(int32_t, i32, float,    f32)
LUX_MAP_DECL_INT_KEY(int32_t, i32, double,   f64)
LUX_MAP_DECL_INT_KEY_STR_VAL(int32_t, i32)

// Int64 key × all values
LUX_MAP_DECL_INT_KEY(int64_t, i64, int8_t,   i8)
LUX_MAP_DECL_INT_KEY(int64_t, i64, int16_t,  i16)
LUX_MAP_DECL_INT_KEY(int64_t, i64, int32_t,  i32)
LUX_MAP_DECL_INT_KEY(int64_t, i64, int64_t,  i64)
LUX_MAP_DECL_INT_KEY(int64_t, i64, uint8_t,  u8)
LUX_MAP_DECL_INT_KEY(int64_t, i64, uint16_t, u16)
LUX_MAP_DECL_INT_KEY(int64_t, i64, uint32_t, u32)
LUX_MAP_DECL_INT_KEY(int64_t, i64, uint64_t, u64)
LUX_MAP_DECL_INT_KEY(int64_t, i64, float,    f32)
LUX_MAP_DECL_INT_KEY(int64_t, i64, double,   f64)
LUX_MAP_DECL_INT_KEY_STR_VAL(int64_t, i64)

// Uint64 key × all values
LUX_MAP_DECL_INT_KEY(uint64_t, u64, int8_t,   i8)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, int16_t,  i16)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, int32_t,  i32)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, int64_t,  i64)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, uint8_t,  u8)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, uint16_t, u16)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, uint32_t, u32)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, uint64_t, u64)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, float,    f32)
LUX_MAP_DECL_INT_KEY(uint64_t, u64, double,   f64)
LUX_MAP_DECL_INT_KEY_STR_VAL(uint64_t, u64)

#endif // LUX_MAP_H
