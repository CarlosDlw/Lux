# Functions

This page covers function declaration, parameters, return values, variadic functions, and function pointers in Lux.

## Declaration

Functions are declared with type-first syntax: return type, name, parameters, and body:

```t
int32 add(int32 a, int32 b) {
    ret a + b;
}

void greet(string name) {
    println(name);
}
```

- The return type comes before the function name.
- Parameters are declared with `type name` syntax.
- Use `ret` to return a value. There is no implicit return.
- Use `void` as the return type when the function returns nothing.

## Calling Functions

```t
namespace CallDemo;

use std::log::println;

int32 square(int32 x) {
    ret x * x;
}

int32 main() {
    int32 result = square(5);
    println(result);    // 25

    ret 0;
}
```

## Return Values

Every non-void function must return a value with `ret`:

```t
int32 factorial(int32 n) {
    if n <= 1 {
        ret 1;
    }
    ret n * factorial(n - 1);
}
```

`void` functions can use `ret;` without a value, or simply let execution reach the end of the function:

```t
void log_if_positive(int32 x) {
    if x <= 0 {
        ret;    // early return, no value
    }
    println(x);
}
```

## Parameters

Parameters use the same type-first syntax as variable declarations:

```t
float64 circle_area(float64 radius) {
    ret 3.14159 * radius * radius;
}

int32 clamp_value(int32 value, int32 min_val, int32 max_val) {
    if value < min_val { ret min_val; }
    if value > max_val { ret max_val; }
    ret value;
}
```

## Variadic Functions

T supports variadic functions — functions that accept a variable number of arguments. The variadic parameter uses `...` before the parameter name:

```t
namespace VariadicDemo;

use std::log::println;

int32 sum(int32 ...values) {
    int32 total = 0;
    for int32 x in values {
        total = total + x;
    }
    ret total;
}

void print_all(int32 ...values) {
    for int32 x in values {
        println(x);
    }
}

int32 main() {
    println(sum(1, 2, 3, 4, 5));    // 15
    println(sum(10, 20));            // 30
    println(sum());                   // 0

    print_all(10, 20, 30);
    // Output: 10 20 30

    ret 0;
}
```

### Variadic Parameter Rules

- The variadic parameter must be the last parameter (or the only one).
- You can have regular parameters before the variadic one:

```t
int32 sum_with_base(int32 base, int32 ...values) {
    int32 total = base;
    for int32 x in values {
        total = total + x;
    }
    ret total;
}

// sum_with_base(100, 1, 2, 3) → 106
```

- Inside the function, the variadic parameter behaves like an array. You can iterate it with `for..in`, access elements by index, and check its length:

```t
int32 first_value(int32 ...values) {
    ret values[0];
}

int32 count_values(int32 ...values) {
    int64 n = values.len;
    ret n;
}
```

## C Variadic Functions

C-style variadic functions (like `printf`) use `...` without a parameter name in `extern` declarations:

```t
extern int32 printf(*char fmt, ...);

int32 main() {
    printf(c"Hello %s, you are %d years old\n", c"Alice", 30);
    ret 0;
}
```

This is different from Lux variadic functions — C variadic arguments are not type-checked and must match the format string.

## Function Pointers

Functions can be referenced by their address and stored in variables:

```t
namespace FnPointerDemo;

use std::log::println;

int32 add(int32 a, int32 b) {
    ret a + b;
}

int32 multiply(int32 a, int32 b) {
    ret a * b;
}

int32 main() {
    fn(int32, int32) -> int32 op = add;
    println(op(3, 4));      // 7

    op = multiply;
    println(op(3, 4));      // 12

    ret 0;
}
```

### Function Type Syntax

The type of a function pointer uses the `fn` keyword:

```t
fn(ParamType1, ParamType2) -> ReturnType
```

Examples:

```t
fn(int32, int32) -> int32       // takes two int32, returns int32
fn(string) -> void              // takes string, returns nothing
fn() -> bool                    // takes nothing, returns bool
```

### Type Aliases for Function Types

Use `type` to create named aliases for function types:

```t
type BinOp = fn(int32, int32) -> int32;
type Predicate = fn(int32) -> bool;
type Action = fn() -> void;
```

### Higher-Order Functions

Functions can accept function pointers as parameters and return them:

```t
namespace HigherOrderDemo;

use std::log::println;

type BinOp = fn(int32, int32) -> int32;

int32 add(int32 a, int32 b) { ret a + b; }
int32 mul(int32 a, int32 b) { ret a * b; }

int32 apply(BinOp op, int32 x, int32 y) {
    ret op(x, y);
}

int32 main() {
    println(apply(add, 3, 4));    // 7
    println(apply(mul, 5, 6));    // 30

    ret 0;
}
```

## Forward Declarations

Functions can be called before they are defined in the same file. The compiler resolves all function declarations before generating code:

```t
namespace ForwardDemo;

use std::log::println;

int32 main() {
    println(double_value(21));    // 42 — works even though defined below
    ret 0;
}

int32 double_value(int32 x) {
    ret x * 2;
}
```

## The `main` Function

Every Lux program must have a `main` function that:

- Returns `int32` (the exit code)
- Takes no parameters, or a single `Vec<string>` parameter for command-line arguments
- Ends with `ret 0;` (or another integer exit code)

```t
int32 main() {
    // program logic
    ret 0;
}
```

### Command-Line Arguments

To receive command-line arguments, declare `main` with a `Vec<string>` parameter:

```t
use std::log::println;
use std::collections::Vec;

int32 main(Vec<string> args) {
    for string arg in args {
        println(arg);
    }

    ret 0;
}
```

The vector contains all arguments passed to the compiled binary, including the program name as the first element.

## See Also

- [Variables](variables.md) — Variable scope in function bodies
- [Types](types.md) — Function type syntax
- [Type Aliases](type-aliases.md) — Named function type aliases
- [Control Flow](control-flow.md) — Control flow within functions
- [Pointers](pointers.md) — Function pointers and address-of
