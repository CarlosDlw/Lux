# Generics

T supports generic type parameters for its built-in collection types and concurrency primitives. Generics allow a single type to work with any element type while maintaining type safety.

---

## Syntax

Generic types are parameterized with angle brackets `<T>`:

```tm
vec<int32> numbers = [1, 2, 3];
map<string, int64> ages;
set<uint64> ids = [];
Task<int32> t = spawn compute(10);
```

---

## Generic Types

T provides four built-in generic types:

| Type | Parameters | Description |
|------|-----------|-------------|
| `vec<T>` | 1 | Dynamic growable array |
| `map<K, V>` | 2 | Open-addressing hash map |
| `set<T>` | 1 | Open-addressing hash set |
| `Task<T>` | 1 | Async task handle |

---

## vec\<T\> — Dynamic Array

A growable, heap-allocated array. Initialized from an array literal or empty `[]`:

```tm
vec<int32> v = [10, 20, 30];
println(v.len());      // 3
println(v.first());    // 10
println(v.last());     // 30

v.push(40);
println(v.len());      // 4

int32 popped = v.pop();
println(popped);       // 40
```

### Index Access

```tm
vec<int32> v = [100, 200, 300];
println(v[0]);    // 100
println(v[2]);    // 300

v[1] = 999;
println(v[1]);    // 999
```

### Vec Methods

| Category | Methods |
|----------|---------|
| **Size** | `len()`, `capacity()`, `isEmpty()` |
| **Access** | `at(usize)`, `first()`, `last()`, `[index]` |
| **Mutation** | `push(T)`, `pop()`, `insert(usize, T)`, `removeAt(usize)`, `removeSwap(usize)`, `clear()`, `fill(T)`, `swap(usize, usize)` |
| **Memory** | `reserve(usize)`, `shrink()`, `resize(usize, T)`, `truncate(usize)`, `free()` |
| **Search** | `contains(T)`, `indexOf(T)`, `lastIndexOf(T)`, `count(T)` |
| **Order** | `reverse()`, `sort()`, `sortDesc()`, `rotate(int32)` |
| **Aggregation** | `sum()`, `product()`, `min()`, `max()`, `average()` |
| **Query** | `equals(vec<T>)`, `isSorted()` |
| **Conversion** | `toString()`, `join(string)`, `clone()` |

```tm
vec<int32> v = [30, 10, 20, 10, 40];

println(v.contains(10));    // true
println(v.indexOf(10));     // 1
println(v.count(10));       // 2

v.sort();
// v is now [10, 10, 20, 30, 40]

println(v.sum());           // 110
println(v.min());           // 10
println(v.max());           // 40

v.free();
```

### Type Flexibility

`vec<T>` works with any type:

```tm
vec<float64> fv = [1.5, 2.5, 3.5];
fv.push(4.5);
println(fv.last());   // 4.5
fv.free();

vec<string> names = ["alice", "bob"];
println(names.first());   // alice
names.free();
```

> **Important:** `vec<T>` is heap-allocated. Call `.free()` when done, or use `defer` for automatic cleanup.

---

## map\<K, V\> — Hash Map

A key-value store using open-addressing hashing:

```tm
map<string, int64> ages;

ages.insert("alice", 30);
ages.insert("bob", 25);
ages.insert("carol", 35);

println(ages.len());          // 3
println(ages.get("alice"));   // 30
println(ages.has("bob"));     // true
```

### Subscript Access

```tm
map<string, int64> ages;
ages["dave"] = 40;
println(ages["dave"]);   // 40
```

### Map Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `len` | `() -> usize` | Number of entries |
| `isEmpty` | `() -> bool` | Whether map has zero entries |
| `get` | `(K) -> V` | Value for key |
| `getOrDefault` | `(K, V) -> V` | Value for key, or default if missing |
| `has` | `(K) -> bool` | Whether key exists |
| `insert` | `(K, V)` | Insert or update entry |
| `remove` | `(K) -> bool` | Remove entry, returns success |
| `clear` | `()` | Remove all entries |
| `keys` | `() -> vec<K>` | All keys as a Vec |
| `values` | `() -> vec<V>` | All values as a Vec |
| `free` | `()` | Release memory |

```tm
map<int32, int64> scores;
scores.insert(1, 100);
scores.insert(2, 200);

println(scores.get(1));              // 100
println(scores.has(3));              // false
println(scores.getOrDefault(4, -1)); // -1

scores.free();
```

---

## set\<T\> — Hash Set

A collection of unique elements using open-addressing hashing:

```tm
set<int32> s = [];

s.add(10);
s.add(20);
s.add(30);
println(s.len());      // 3

bool dup = s.add(20);
println(dup);           // false (already exists)
println(s.len());       // 3 (unchanged)
```

### Set Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `len` | `() -> usize` | Number of elements |
| `isEmpty` | `() -> bool` | Whether set is empty |
| `add` | `(T) -> bool` | Add element, returns `true` if new |
| `has` | `(T) -> bool` | Whether element exists |
| `remove` | `(T) -> bool` | Remove element, returns success |
| `clear` | `()` | Remove all elements |
| `values` | `() -> set<T>` | Copy of the set |
| `free` | `()` | Release memory |

```tm
set<string> ss = [];
ss.add("hello");
ss.add("world");
ss.add("hello");       // returns false — duplicate

println(ss.len());     // 2
println(ss.has("hello")); // true
println(ss.has("foo"));   // false

ss.remove("world");
println(ss.len());     // 1

ss.free();
```

---

## Task\<T\> — Async Task

Created by the `spawn` operator and resolved with `await`. See [Concurrency](concurrency.md) for full details.

```tm
use std::thread::Task;

Task<int32> t = spawn compute(10, 20);
int32 result = await t;
```

---

## Memory Management

`vec<T>`, `map<K, V>`, and `set<T>` are heap-allocated. They must be freed when no longer needed:

```tm
vec<int32> v = [1, 2, 3];
defer v.free();

// use v normally...
// v.free() is called automatically at function exit
```

The compiler also performs automatic cleanup for these types when they go out of scope at function exit, but using `defer` makes the intent explicit.

---

## See Also

- [Arrays](arrays.md) — Fixed-size stack-allocated arrays
- [Type Aliases](type-aliases.md) — `type` keyword for type shortcuts
- [Concurrency](concurrency.md) — `spawn`, `await`, `Task<T>`
