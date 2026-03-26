# Operator Precedence

Complete table of operator precedence from **highest** (tightest binding) to **lowest**. Operators in the same level have equal precedence and are evaluated left-to-right unless noted.

---

## Precedence Table

| Level | Operator(s) | Description | Associativity |
|-------|-------------|-------------|---------------|
| 17 | `!` `-` (unary) `~` `*` `&` | Unary: not, negate, bitwise not, dereference, address-of | Right |
| 16 | `.` `->` `()` `[]` | Member access, arrow access, function call, index | Left |
| 15 | `as` | Type cast | Left |
| 14 | `**` | Exponentiation | Right |
| 13 | `*` `/` `%` | Multiplication, division, modulo | Left |
| 12 | `+` `-` | Addition, subtraction | Left |
| 11 | `<<` `>>` | Bitwise shift left, shift right | Left |
| 10 | `&` | Bitwise AND | Left |
| 9 | `^` | Bitwise XOR | Left |
| 8 | `\|` | Bitwise OR | Left |
| 7 | `..` `..=` | Range (exclusive), range (inclusive) | Left |
| 6 | `<` `<=` `>` `>=` | Relational comparison | Left |
| 5 | `==` `!=` | Equality, inequality | Left |
| 4 | `&&` | Logical AND (short-circuit) | Left |
| 3 | `\|\|` | Logical OR (short-circuit) | Left |
| 2 | `??` | Null coalescing | Left |
| 1 | `? :` | Ternary conditional | Right |

---

## Unary Operators (Level 17)

| Operator | Example | Description |
|----------|---------|-------------|
| `!` | `!flag` | Logical NOT |
| `-` | `-x` | Arithmetic negation |
| `~` | `~bits` | Bitwise complement |
| `*` | `*ptr` | Pointer dereference |
| `&` | `&value` | Address-of |

```tm
let x: int32 = -5
let positive: bool = !false
let mask: uint8 = ~0u8        // 0xFF
let ptr: *int32 = &x
let val: int32 = *ptr
```

---

## Postfix Operators (Level 16)

| Operator | Example | Description |
|----------|---------|-------------|
| `.` | `obj.field` | Member access |
| `->` | `ptr->field` | Arrow (pointer member access) |
| `()` | `func(args)` | Function call |
| `[]` | `arr[i]` | Index |

```tm
let name: string = person.name
let val: int32 = ptr->value
let result: int32 = add(2, 3)
let first: int32 = numbers[0]
```

---

## Type Cast (Level 15)

```tm
let x: int32 = 42
let f: float64 = x as float64       // 42.0
let byte: uint8 = 256 as uint8      // truncation: 0
```

---

## Arithmetic (Levels 14–12)

| Level | Operators | Example |
|-------|-----------|---------|
| 14 | `**` | `2 ** 10` → 1024 |
| 13 | `* / %` | `10 / 3` → 3, `10 % 3` → 1 |
| 12 | `+ -` | `a + b - c` |

```tm
let result: int32 = 2 + 3 * 4      // 14, not 20
let exp: int32 = 2 ** 3 ** 2        // 512 (right-assoc: 2^(3^2))
```

---

## Bitwise (Levels 11–8)

| Level | Operator | Name |
|-------|----------|------|
| 11 | `<< >>` | Shift |
| 10 | `&` | AND |
| 9 | `^` | XOR |
| 8 | `\|` | OR |

```tm
let flags: uint8 = 0b1010
let masked: uint8 = flags & 0x0F
let shifted: uint8 = 1u8 << 4       // 0b10000
let combined: uint8 = 0x0F | 0xF0   // 0xFF
```

---

## Range (Level 7)

```tm
for i in 0..10 {       // 0 to 9
    // ...
}

for i in 1..=100 {     // 1 to 100 inclusive
    // ...
}
```

---

## Comparison (Levels 6–5)

```tm
let less: bool = a < b
let equal: bool = x == y
let inRange: bool = x >= 0 && x < 100
```

---

## Logical (Levels 4–3)

Both `&&` and `||` use **short-circuit evaluation**: the right operand is only evaluated if needed.

```tm
let valid: bool = x > 0 && y > 0     // y > 0 skipped if x <= 0
let fallback: bool = a || b           // b skipped if a is true
```

---

## Null Coalescing (Level 2)

```tm
let port: int32 = config.port ?? 8080
```

---

## Ternary Conditional (Level 1)

Right-associative. Lowest precedence among operators.

```tm
let label: string = count == 1 ? "item" : "items"
let clamped: int32 = x < 0 ? 0 : x > 100 ? 100 : x
```

---

## Compound Assignment Operators

Compound assignments are **statements**, not expressions. They combine an operator with assignment.

| Operator | Equivalent |
|----------|-----------|
| `+=` | `x = x + rhs` |
| `-=` | `x = x - rhs` |
| `*=` | `x = x * rhs` |
| `/=` | `x = x / rhs` |
| `%=` | `x = x % rhs` |
| `&=` | `x = x & rhs` |
| `\|=` | `x = x \| rhs` |
| `^=` | `x = x ^ rhs` |
| `<<=` | `x = x << rhs` |
| `>>=` | `x = x >> rhs` |

```tm
var count: int32 = 0
count += 1
count *= 2
count >>= 1
```

---

## Operator Overloading

T does **not** support operator overloading. All operators work only on built-in types.
