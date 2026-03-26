# Map\<K, V\>

An open-addressing hash map with linear probing.

## Import

```tm
use std::collections::Map;
```

## Construction

```tm
Map<string, int32> ages = [];
```

## Methods

### Size & Query

| Method | Returns | Description |
|--------|---------|-------------|
| `len()` | `usize` | Number of key-value pairs |
| `isEmpty()` | `bool` | Whether map is empty |

### Lookup

| Method | Returns | Description |
|--------|---------|-------------|
| `get(K)` | `V` | Get value for key |
| `getOrDefault(K, V)` | `V` | Get value or return default |
| `has(K)` | `bool` | Whether key exists |

### Mutation

| Method | Returns | Description |
|--------|---------|-------------|
| `set(K, V)` | `void` | Insert or update key-value pair |
| `remove(K)` | `bool` | Remove key, returns whether it existed |
| `clear()` | `void` | Remove all entries |

### Iteration

| Method | Returns | Description |
|--------|---------|-------------|
| `keys()` | `Vec<K>` | All keys as vector |
| `values()` | `Vec<V>` | All values as vector |

### Cleanup

| Method | Returns | Description |
|--------|---------|-------------|
| `free()` | `void` | Free memory (auto-called at scope exit) |

## Example

```tm
use std::log::println;

Map<string, int32> scores = [];

scores.set("alice", 95);
scores.set("bob", 87);
scores.set("carol", 92);

println(scores.len());                    // 3
println(scores.has("alice"));             // true
println(scores.get("alice"));             // 95
println(scores.getOrDefault("dave", 0));  // 0

scores.remove("bob");
println(scores.len());                    // 2

Vec<string> k = scores.keys();
Vec<int32> v = scores.values();
```

## Auto-Cleanup

`Map<K, V>` memory is automatically freed when the containing function returns. You can also call `.free()` manually or use `defer`.

## See Also

- [Vec\<T\>](vec.md) — Dynamic array
- [Set\<T\>](set.md) — Unique value set
- [Generics](../../language/generics.md) — Generic types overview
