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
