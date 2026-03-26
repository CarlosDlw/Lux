# CLI Usage

This page documents all command-line options for the Lux compiler (`lux`).

## Synopsis

```
lux <file.lx>              Compile and print LLVM IR to stdout
lux <file.lx> <output>     Compile and emit native binary
lux <file.lx> <output> -oN Compile with optimization level N
lux helpc <lib> [symbol]   C library reference helper
lux help                   Show help message
lux version                Show compiler version
```

## Arguments

### Input File (Required)

The first positional argument is the path to a `.lx` source file:

```bash
lux main.lx
lux examples/hello.lx
lux src/app.lx
```

The compiler scans the directory tree containing this file to discover all `.lx` modules (for namespace resolution and multi-file projects).

### Output File (Optional)

The second positional argument is the path for the compiled native binary:

```bash
lux main.lx ./main
lux src/app.lx build/app
```

If omitted, the compiler prints the generated LLVM IR to stdout instead of producing a binary:

```bash
lux main.lx              # prints IR to stdout
lux main.lx > output.ll  # redirect IR to a file
```

## Flags

### Optimization Level

```
-o1    Basic optimizations (constant folding, dead code elimination)
-o2    Standard optimizations (inlining, loop optimizations) — recommended
-o3    Aggressive optimizations (vectorization, full LTO)
```

The default optimization level is 0 (no optimization). Use `-o2` for production builds:

```bash
lux main.lx ./main -o2
```

### Linker Flags

```
-lname    Link against library 'name'
```

Links the output binary against a system or third-party library. The flag is passed directly to the system linker.

```bash
lux app.lx ./app -lSDL2           # link against SDL2
lux app.lx ./app -lm -lpthread    # link against math and pthreads
```

### Library Search Paths

```
-Lpath    Add 'path' to the library search path
```

Adds a directory where the linker looks for libraries specified with `-l`:

```bash
lux app.lx ./app -L/opt/lib -lmylib
lux app.lx ./app -L./vendor/lib -lvendor
```

### Include Search Paths

```
-Ipath    Add 'path' to the C header include search path
```

Adds a directory where the compiler looks for C header files used with `#include`:

```bash
lux app.lx ./app -I/opt/include -lmylib
lux app.lx ./app -I./vendor/include -L./vendor/lib -lvendor
```

### Help and Version

```bash
lux help        # or --help, -h
lux version     # or --version, -v
```

## Examples

### Print LLVM IR (Debugging)

```bash
lux main.lx
```

Useful for inspecting the generated IR without producing a binary. The output can be piped or redirected:

```bash
lux main.lx | less
lux main.lx > debug.ll
```

### Simple Compilation

```bash
lux main.lx ./main
./main
```

### Optimized Build

```bash
lux main.lx ./main -o2
./main
```

### Linking with External C Libraries

```bash
# Link against SDL2
lux game.lx ./game -lSDL2

# Link against a custom library in a non-standard path
lux app.lx ./app -L/opt/custom/lib -I/opt/custom/include -lcustom

# Link against multiple libraries
lux server.lx ./server -lssl -lcrypto -lpthread
```

### Using Make

The project Makefile provides a shortcut for building and running:

```bash
make run FILE=examples/main.lx
```

## Build Artifacts

When compiling to a binary, the compiler creates a `.luxbuild/` directory in the project root. This directory contains intermediate object files (`.o`) generated during compilation:

```
project/
├── main.lx
├── utils.lx
└── .luxbuild/
    ├── Main__main.o
    ├── Utils__utils.o
    └── c__helper.o      # if local C files are included
```

These files are reused across compilations. You can safely delete `.luxbuild/` to force a clean rebuild.

## Compiler Pipeline

When you run `lux main.lx ./main`, the compiler performs these steps in order:

1. **Scan** — Discover all `.lx` files in the project directory
2. **Parse** — Parse each file using the ANTLR4 grammar
3. **Register** — Build a namespace registry for cross-file module resolution
4. **Resolve Headers** — Process `#include` directives (system and local C headers)
5. **Compile C** — Compile any local `.c` files referenced by `#include` to object files
6. **Check** — Run semantic analysis and type checking on all Lux code
7. **Generate IR** — Emit LLVM intermediate representation
8. **Optimize** — Apply LLVM optimization passes (if `-oN` is specified)
9. **Emit Objects** — Write `.o` object files to `.luxbuild/`
10. **Link** — Invoke the system linker to produce the final native binary

## Error Messages

The compiler produces clear error messages. Some common ones:

```
lux: unknown flag '-xyz'
lux: Run 'lux help' for usage.
```

Unknown command-line flag. Check the flag spelling.

```
lux: no .lx files found in '/path/to/project'
```

No Lux source files found in the directory containing your input file.

```
lux: file 'module.lx' is missing a 'namespace' declaration
```

Every `.lx` file must begin with a `namespace` declaration.

```
lux: cannot find header '<mylib.h>'. Check '-I' include paths
```

A `#include` directive references a header that cannot be found. Add the correct `-I` path.

## Subcommands

### helpc — C Library Reference

```bash
lux helpc <lib> [symbol] [flags]
```

A built-in tool that inspects C library headers and displays detailed information
about functions, structs, enums, macros, and typedefs. Maps every C type to its
Lux equivalent.

```bash
lux helpc raylib InitWindow     # look up a function
lux helpc raylib Color          # look up a struct
lux helpc raylib --list         # list all symbols
lux helpc raylib -s Draw        # search by name
lux helpc stdio printf          # works with any installed header
lux helpc math sin --json       # JSON output
```

See [helpc](../tools/helpc.md) for the full reference.

## See Also

- [Installation](installation.md) — Build the compiler from source
- [Hello World](hello-world.md) — Your first Lux program
- [Linking](../ffi/linking.md) — Detailed guide on linking with C libraries
