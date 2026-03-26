# Type Aliases

The `type` keyword creates an alias — a new name for an existing type. This is especially useful for function pointer types and improving code readability.

---

## Syntax

```
type Name = existingType;
```

---

## Function Type Aliases

The most common use case is naming function pointer types:

```tm
type BinOp = fn(int32, int32) -> int32;
```

This creates `BinOp` as a shorthand for a function that takes two `int32` parameters and returns `int32`.

### Using Function Type Aliases

```tm
type BinOp = fn(int32, int32) -> int32;

int32 add(int32 a, int32 b) {
    ret a + b;
}

int32 mul(int32 a, int32 b) {
    ret a * b;
}

int32 apply(BinOp op, int32 x, int32 y) {
    ret op(x, y);
}

int32 main() {
    BinOp op = add;
    println(op(3, 4));          // 7

    println(apply(mul, 5, 6));  // 30

    op = mul;
    println(op(3, 4));          // 12

    ret 0;
}
```

Type aliases make function pointer signatures readable when used as parameters, return types, or variable types.

---

## Built-in Type Alias: `cstring`

T provides a built-in type alias:

```
cstring = *char
```

This is used for C string interop:

```tm
cstring greeting = c"Hello!";
puts(greeting);

cstring converted = cstr("from T string");
puts(converted);
free(converted as *void);
```

`cstring` and `*char` are fully interchangeable — they are the same type.

---

## See Also

- [Functions](functions.md) — Function declarations and function pointers
- [Types](types.md) — Primitive type reference
- [Generics](generics.md) — Parameterized types
