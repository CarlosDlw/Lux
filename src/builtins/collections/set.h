#ifndef LUX_SET_H
#define LUX_SET_H

#include <stddef.h>
#include <stdint.h>

// ═══════════════════════════════════════════════════════════════════════════
// Set<T> — Open-addressing hash set
//
// LLVM struct: { ptr states, ptr keys, ptr hashes,
//                usize len, usize cap, usize key_size }
// C struct mirrors LLVM layout exactly.
// ═══════════════════════════════════════════════════════════════════════════

typedef struct {
    const char* ptr;
    size_t      len;
} lux_set_string;

typedef struct {
    uint8_t*  states;     // 0=empty, 1=occupied, 2=tombstone
    void*     keys;       // flat key array (key_size * cap bytes)
    uint64_t* hashes;     // cached hashes per slot
    size_t    len;        // active entries
    size_t    cap;        // total slots
    size_t    key_size;   // bytes per key
} lux_set_header;

// Vec out-parameter (mirrors lux_vec_header layout)
typedef struct {
    void*  ptr;
    size_t len;
    size_t cap;
} lux_set_vec_out;

// ── Macro-generated declarations ─────────────────────────────────────────

// For integer element types: ET = C type, ES = suffix
#define LUX_SET_DECL_INT(ET, ES)                                          \
void   lux_set_init_##ES(lux_set_header* s);                           \
void   lux_set_free_##ES(lux_set_header* s);                           \
size_t lux_set_len_##ES(const lux_set_header* s);                      \
int    lux_set_isEmpty_##ES(const lux_set_header* s);                  \
int    lux_set_add_##ES(lux_set_header* s, ET elem);                   \
int    lux_set_has_##ES(lux_set_header* s, ET elem);                   \
int    lux_set_remove_##ES(lux_set_header* s, ET elem);                \
void   lux_set_clear_##ES(lux_set_header* s);                          \
void   lux_set_values_##ES(lux_set_header* s, lux_set_vec_out* out);

// For string element type
#define LUX_SET_DECL_STR()                                                \
void   lux_set_init_str(lux_set_header* s);                            \
void   lux_set_free_str(lux_set_header* s);                            \
size_t lux_set_len_str(const lux_set_header* s);                       \
int    lux_set_isEmpty_str(const lux_set_header* s);                   \
int    lux_set_add_str(lux_set_header* s, lux_set_string elem);     \
int    lux_set_has_str(lux_set_header* s, lux_set_string elem);     \
int    lux_set_remove_str(lux_set_header* s, lux_set_string elem);  \
void   lux_set_clear_str(lux_set_header* s);                           \
void   lux_set_values_str(lux_set_header* s, lux_set_vec_out* out);

// ── Instantiate declarations ─────────────────────────────────────────────

// Integer element types
LUX_SET_DECL_INT(int8_t,   i8)
LUX_SET_DECL_INT(int16_t,  i16)
LUX_SET_DECL_INT(int32_t,  i32)
LUX_SET_DECL_INT(int64_t,  i64)
LUX_SET_DECL_INT(uint8_t,  u8)
LUX_SET_DECL_INT(uint16_t, u16)
LUX_SET_DECL_INT(uint32_t, u32)
LUX_SET_DECL_INT(uint64_t, u64)

// String element type
LUX_SET_DECL_STR()

// ── Raw (opaque struct) element variant ───────────────────────────────
// Used when element is a user-defined struct (builtinSuffix == "raw").
// Hashing is byte-level FNV-1a; equality is bitwise memcmp.
void   lux_set_init_raw(lux_set_header* s, size_t elem_size);
void   lux_set_free_raw(lux_set_header* s);
size_t lux_set_len_raw(const lux_set_header* s);
int    lux_set_add_raw(lux_set_header* s, const void* elem);
int    lux_set_has_raw(lux_set_header* s, const void* elem);
int    lux_set_remove_raw(lux_set_header* s, const void* elem);
void   lux_set_values_raw(lux_set_header* s, lux_set_vec_out* out);
int    lux_set_isEmpty_raw(const lux_set_header* s);
void   lux_set_clear_raw(lux_set_header* s);

#endif // LUX_SET_H
