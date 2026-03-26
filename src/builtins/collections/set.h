#ifndef TOLLVM_SET_H
#define TOLLVM_SET_H

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
} tollvm_set_string;

typedef struct {
    uint8_t*  states;     // 0=empty, 1=occupied, 2=tombstone
    void*     keys;       // flat key array (key_size * cap bytes)
    uint64_t* hashes;     // cached hashes per slot
    size_t    len;        // active entries
    size_t    cap;        // total slots
    size_t    key_size;   // bytes per key
} tollvm_set_header;

// Vec out-parameter (mirrors tollvm_vec_header layout)
typedef struct {
    void*  ptr;
    size_t len;
    size_t cap;
} tollvm_set_vec_out;

// ── Macro-generated declarations ─────────────────────────────────────────

// For integer element types: ET = C type, ES = suffix
#define TOLLVM_SET_DECL_INT(ET, ES)                                          \
void   tollvm_set_init_##ES(tollvm_set_header* s);                           \
void   tollvm_set_free_##ES(tollvm_set_header* s);                           \
size_t tollvm_set_len_##ES(const tollvm_set_header* s);                      \
int    tollvm_set_isEmpty_##ES(const tollvm_set_header* s);                  \
int    tollvm_set_add_##ES(tollvm_set_header* s, ET elem);                   \
int    tollvm_set_has_##ES(tollvm_set_header* s, ET elem);                   \
int    tollvm_set_remove_##ES(tollvm_set_header* s, ET elem);                \
void   tollvm_set_clear_##ES(tollvm_set_header* s);                          \
void   tollvm_set_values_##ES(tollvm_set_header* s, tollvm_set_vec_out* out);

// For string element type
#define TOLLVM_SET_DECL_STR()                                                \
void   tollvm_set_init_str(tollvm_set_header* s);                            \
void   tollvm_set_free_str(tollvm_set_header* s);                            \
size_t tollvm_set_len_str(const tollvm_set_header* s);                       \
int    tollvm_set_isEmpty_str(const tollvm_set_header* s);                   \
int    tollvm_set_add_str(tollvm_set_header* s, tollvm_set_string elem);     \
int    tollvm_set_has_str(tollvm_set_header* s, tollvm_set_string elem);     \
int    tollvm_set_remove_str(tollvm_set_header* s, tollvm_set_string elem);  \
void   tollvm_set_clear_str(tollvm_set_header* s);                           \
void   tollvm_set_values_str(tollvm_set_header* s, tollvm_set_vec_out* out);

// ── Instantiate declarations ─────────────────────────────────────────────

// Integer element types
TOLLVM_SET_DECL_INT(int8_t,   i8)
TOLLVM_SET_DECL_INT(int16_t,  i16)
TOLLVM_SET_DECL_INT(int32_t,  i32)
TOLLVM_SET_DECL_INT(int64_t,  i64)
TOLLVM_SET_DECL_INT(uint8_t,  u8)
TOLLVM_SET_DECL_INT(uint16_t, u16)
TOLLVM_SET_DECL_INT(uint32_t, u32)
TOLLVM_SET_DECL_INT(uint64_t, u64)

// String element type
TOLLVM_SET_DECL_STR()

#endif // TOLLVM_SET_H
