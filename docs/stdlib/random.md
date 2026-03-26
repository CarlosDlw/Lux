# std::random

Pseudorandom number generation functions.

## Import

```tm
use std::random::{ seed, seedTime, randInt, randFloat, randBool };
```

## Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `seed` | `(int64) -> void` | Set PRNG seed to specific value |
| `seedTime` | `() -> void` | Seed PRNG from current time |
| `randInt` | `() -> int32` | Random 32-bit integer |
| `randIntRange` | `(int32, int32) -> int32` | Random integer in [min, max) |
| `randUint` | `() -> uint32` | Random unsigned 32-bit integer |
| `randFloat` | `() -> float64` | Random float in [0.0, 1.0) |
| `randFloatRange` | `(float64, float64) -> float64` | Random float in [min, max) |
| `randBool` | `() -> bool` | Random boolean |
| `randChar` | `() -> char` | Random printable ASCII character |

## Example

```tm
use std::random::{ seedTime, randInt, randIntRange, randFloat, randBool };
use std::log::println;

seedTime();

println(randInt());                // -1234567890
println(randIntRange(1, 100));     // 42
println(randFloat());              // 0.731284
println(randBool());               // true
println(randIntRange(0, 6) + 1);   // dice roll: 1-6
```

> **Note:** Call `seedTime()` or `seed(n)` before generating random numbers. Without seeding, output is deterministic.

## See Also

- [std::math](math.md) — Mathematical functions
- [std::crypto](crypto.md) — Cryptographic random bytes
