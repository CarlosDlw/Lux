# Memory Management

T uses manual memory management with three complementary mechanisms: `defer` for explicit cleanup scheduling, automatic cleanup for collection types, and the `std::mem` module for low-level allocation.

---

## `defer`

The `defer` statement schedules code to execute when the current function exits. Deferred statements run in **LIFO order** (last deferred runs first).

### Basic Usage

```tm
#include <stdlib.h>

void example() {
    *int32 ptr = malloc(4) as *int32;
    defer free(ptr as *void);

    *ptr = 42;
    println(*ptr);   // 42
    // free(ptr) runs here automatically
}
```

### LIFO Execution Order

```tm
void example() {
    defer println(1);
    defer println(2);
    defer println(3);
    println(0);
}
// Output: 0, 3, 2, 1
```

### With Any Statement

`defer` works with any statement or expression:

```tm
defer close(fd);
defer v.free();
defer println("cleanup done");
```

---

## Automatic Cleanup for Collections

`vec<T>`, `map<K, V>`, and `set<T>` are automatically freed when the function they're declared in exits. This prevents memory leaks for the common case of local collections.

### Vec Auto-Cleanup

```tm
void example() {
    vec<int32> v;
    v.push(10);
    v.push(20);
    v.push(30);
    println(v.len());   // 3
    // v is automatically freed at function exit
}
```

### Map Auto-Cleanup

```tm
void example() {
    map<int32, int32> m;
    m.insert(1, 100);
    m.insert(2, 200);
    println(m.len());   // 2
    // m is automatically freed at function exit
}
```

### Set Auto-Cleanup

```tm
void example() {
    set<int32> s;
    s.add(1);
    s.add(2);
    s.add(3);
    println(s.len());   // 3
    // s is automatically freed at function exit
}
```

### Mixed Defer and Auto-Cleanup

Both mechanisms can coexist in the same function. Explicit `defer` statements run first (LIFO), then auto-cleanup runs:

```tm
void example() {
    vec<int32> v;
    v.push(1);
    defer println(99);
    vec<int32> w;
    w.push(2);
    w.push(3);
    println(v.len());   // 1
    println(w.len());   // 2
    // defer println(99) runs first
    // then v and w are auto-freed
}
```

### Explicit `.free()`

You can also call `.free()` manually or use `defer` for explicit control:

```tm
vec<int32> v = [1, 2, 3];
defer v.free();
// ... use v ...
```

---

## `std::mem` — Low-Level Memory Operations

For direct heap allocation and manipulation, import from `std::mem`:

```tm
use std::mem::alloc;
use std::mem::allocZeroed;
use std::mem::realloc;
use std::mem::free;
use std::mem::copy;
use std::mem::move;
use std::mem::set;
use std::mem::zero;
use std::mem::compare;
```

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `alloc` | `(usize) -> *void` | Allocate bytes (uninitialized) |
| `allocZeroed` | `(usize) -> *void` | Allocate zeroed bytes |
| `realloc` | `(*void, usize) -> *void` | Resize allocation |
| `free` | `(*void)` | Free allocation |
| `copy` | `(*void dst, *void src, usize n)` | Copy n bytes (non-overlapping) |
| `move` | `(*void dst, *void src, usize n)` | Copy n bytes (overlapping-safe) |
| `set` | `(*void, int32 byte, usize n)` | Fill n bytes with value |
| `zero` | `(*void, usize n)` | Zero out n bytes |
| `compare` | `(*void, *void, usize) -> int32` | Compare n bytes (like `memcmp`) |

### Example

```tm
use std::mem::{ alloc, allocZeroed, copy, compare, free };

*int32 p = alloc(4);
*p = 42;
println(*p);            // 42

*int32 q = allocZeroed(4);
println(*q);            // 0

copy(q, p, 4);
println(*q);            // 42

int32 cmp = compare(p, q, 4);
println(cmp);           // 0 (equal)

free(p);
free(q);
```

---

## C `malloc` / `free`

You can also use C's `malloc` and `free` directly via `#include` or `extern`:

```tm
#include <stdlib.h>

*int32 ptr = malloc(4) as *int32;
*ptr = 42;
free(ptr as *void);
```

Or with extern declarations:

```tm
extern *void malloc(usize size);
extern void free(*void ptr);
```

---

## Summary: When to Use What

| Mechanism | Use Case |
|-----------|----------|
| Auto-cleanup | Local `Vec`, `Map`, `Set` — no action needed |
| `defer` | Explicit cleanup of raw pointers, file handles, etc. |
| `v.free()` | Manual early cleanup of collections |
| `std::mem` | Low-level allocation, copy, compare |
| C `malloc`/`free` | Direct C interop |

---

## See Also

- [Pointers](pointers.md) — Pointer types, address-of, dereference
- [Generics](generics.md) — `vec<T>`, `map<K,V>`, `set<T>` methods
- [Modules](modules.md) — Importing `std::mem`
