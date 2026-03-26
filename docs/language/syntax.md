# Syntax

This page covers the general syntax rules of T: how statements are structured, how blocks work, and how the language handles comments, identifiers, and whitespace.

## Statements and Semicolons

Every statement in Lux ends with a semicolon:

```t
int32 x = 10;
println(x);
ret 0;
```

Block statements (`if`, `for`, `while`, `switch`, `lock`, `try`) do **not** end with a semicolon — the closing brace ends the statement:

```t
if x > 0 {
    println(x);
}
```

## Blocks

Blocks use curly braces `{}`. Braces are always required — there are no single-statement shortcuts:

```t
// correct
if x > 0 {
    println(x);
}

// this is NOT valid
// if x > 0 println(x);
```

## Comments

T supports two comment styles:

```t
// Line comment — everything after // to end of line

/* Block comment — can span
   multiple lines */
```

Block comments do not nest.

## Identifiers

Identifiers follow standard rules: they start with a letter or underscore, followed by letters, digits, or underscores:

```t
int32 count = 0;
string user_name = "Alice";
float64 _temp = 98.6;
bool isReady = true;
```

Lux is case-sensitive: `count`, `Count`, and `COUNT` are three different identifiers.

## Namespaces

Every `.lx` file must begin with a namespace declaration:

```t
namespace MyModule;
```

The namespace name is used for module resolution when importing symbols across files. It must be the first non-comment line in the file.

## Imports

After the namespace, you can import symbols from the standard library or other modules:

```t
use std::log::println;                    // import a single function
use std::log::{ println, print, dbg };    // import multiple functions
use std::math;                            // import an entire module
```

Imports must appear after the namespace declaration and before any other declarations.

## Include Directives

C headers can be included for FFI:

```t
#include <stdio.h>        // system header
#include "mylib.h"        // local header
```

Include directives are processed before compilation and make C functions, structs, enums, and constants available in the T file.

## Type-First Declarations

All declarations in Lux place the type before the name:

```t
int32 age = 25;                               // variable
float64 calculate_area(float64 radius) { }    // function
struct Point { int32 x; int32 y; }            // struct fields
```

This applies consistently to variables, function parameters, function return types, and struct fields.

## Function Syntax

Functions are declared with the return type, name, parameters, and body:

```t
int32 add(int32 a, int32 b) {
    ret a + b;
}

void greet(string name) {
    println(name);
}
```

The `ret` keyword is used to return values. There is no implicit return.

## Optional Parentheses

Control flow statements (`if`, `while`, `switch`) accept conditions with or without parentheses:

```t
// both are valid
if x > 0 { println(x); }
if (x > 0) { println(x); }

while x > 0 { x -= 1; }
while (x > 0) { x -= 1; }

switch x { case 1 { } }
switch (x) { case 1 { } }
```

## Entry Point

The program entry point is the `main` function, which must return `int32`:

```t
int32 main() {
    // program starts here
    ret 0;
}
```

## Escape Sequences

String and character literals support these escape sequences:

| Sequence | Meaning |
|---|---|
| `\n` | Newline |
| `\t` | Tab |
| `\r` | Carriage return |
| `\\` | Backslash |
| `\"` | Double quote |
| `\'` | Single quote |
| `\0` | Null character |
| `\a` | Bell |
| `\xNN` | Hex byte (e.g., `\x41` = `'A'`) |

## See Also

- [Overview](overview.md) — Language philosophy and goals
- [Types](types.md) — All built-in types
- [Variables](variables.md) — Variable declaration and scope
