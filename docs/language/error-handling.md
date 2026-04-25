# Error Handling

T provides structured error handling with `try`/`catch`/`finally`, `throw`, and the built-in `Error` struct. For immediate program termination, global builtins `panic`, `assert`, `assertMsg`, and `unreachable` are also available.

---

## The Error Struct

When an error is thrown, it is represented as an `Error` struct with four fields:

| Field | Type | Description |
|-------|------|-------------|
| `message` | `string` | Error description |
| `file` | `string` | Source file where the error was thrown |
| `line` | `int32` | Line number |
| `column` | `int32` | Column number |

The `file`, `line`, and `column` fields are automatically filled by the compiler at the `throw` site.

---

## throw

The `throw` statement raises an error. Only the `message` field needs to be provided:

```tm
throw Error { message: "something went wrong" };
```

The remaining fields (`file`, `line`, `column`) are injected automatically.

---

## try / catch / finally

The `try` block wraps code that may throw. The `catch` block handles the error. The `finally` block runs regardless of whether an error occurred.

```tm
try {
    println("before throw");
    throw Error { message: "something went wrong" };
    println("should not print");
} catch (Error e) {
    println(e.message);   // "something went wrong"
    println(e.file);      // source file path
    println(e.line);      // line number of the throw
    println(e.column);    // column number
} finally {
    println("finally block");
}

println("after try/catch");
```

**Output:**
```
before throw
something went wrong
<file path>
<line>
<column>
finally block
after try/catch
```

### Block combinations

All three blocks can be combined:

| Form | Valid |
|------|-------|
| `try { } catch (Error e) { }` | Yes |
| `try { } finally { }` | Yes |
| `try { } catch (Error e) { } finally { }` | Yes |

### Execution flow

1. Code in `try` executes normally
2. If `throw` is reached, execution jumps to `catch`
3. `finally` always executes — after `try` completes normally, or after `catch` handles the error
4. Execution continues after the entire `try`/`catch`/`finally` block

---

## Enum Unwrap with catch

`expr catch { ... }` is an expression-level unwrap for enum-based error returns.

Use it when a function returns an enum shaped like success-or-error, and you want:

- the success payload as the value of the expression
- an inline error branch with access to the `Error` payload

### Basic form

```tm
auto value = divide(10, 0) catch {
    println(it.message);
    ret 1;
};
```

### What happens

1. `divide(10, 0)` is evaluated.
2. If it is the success variant, the success payload becomes the value of the full expression.
3. If it is the error variant, the `catch` block runs.
4. Inside that block, `it` is the implicit `Error` payload.

### Required enum shape

`expr catch { ... }` is accepted only if the expression type is an enum with this exact structure:

1. Exactly 2 variants total.
2. One variant has exactly one payload and this payload type is `Error`.
3. The other variant has exactly one payload of any type (the success payload).
4. Variant names do not matter. Only shape and payload types are checked.

### Type inference

The unwrap expression type is the success payload type.

```tm
auto value = divide(10, 2) catch {
    ret 1;
};
// value is inferred as int32 if success payload is int32
```

### Scope of `it`

- `it` exists only inside the `catch` block of `expr catch { ... }`.
- Using `it` outside this block is a checker error.

### Valid and invalid patterns

Valid:

```tm
enum Response {
    Ok(int32),
    Err(Error)
}
```

Also valid (different names, same shape):

```tm
enum OperationResult {
    Success(string),
    Failure(Error)
}
```

Invalid (more than 2 variants):

```tm
enum Bad {
    Ok(int32),
    Err(Error),
    Unknown(Error)
}
```

Invalid (error payload is not exactly `Error`):

```tm
enum Bad {
    Ok(int32),
    Err(string)
}
```

### Complete example (from examples/main.lx)

```tm
namespace Main;

#include <stdlib.h>
#include <stdio.h>

enum Response {
    Ok(int32),
    Err(Error)
}

Response divide(int32 a, int32 b) {
    try {
        if b == 0 {
            ret Response::Err(Error { message: "Division by zero!" });
        }

        ret Response::Ok(a / b);
    } catch(Error e) {
        ret Response::Err(e);
    }
}

int32 main() {
    auto value = divide(10, 0) catch {
        printf(c"Error: %s\\n", it.message);
        ret 1;
    };

    printf(c"%d\\n", value);
    ret 0;
}
```

---

## Global Error Builtins

These functions are available without any `use` import and immediately terminate the program:

### `panic(string)`

Aborts with an error message:

```tm
panic("something went wrong!");
```

Prints the message to stderr and exits with a non-zero code.

### `assert(bool)`

Aborts if the condition is `false`:

```tm
assert(x > 0);
```

### `assertMsg(bool, string)`

Aborts if the condition is `false`, with a custom message:

```tm
assertMsg(x > 0, "expected positive value");
```

### `unreachable()`

Marks code that should never be reached. If executed, the program aborts:

```tm
unreachable();
```

---

## See Also

- [Control Flow](control-flow.md) — `try`/`catch`/`finally` in the context of control flow
- [Modules](modules.md) — Global builtins available without imports
