# CLI Usage

This page documents all command-line options for the T compiler (`tollvm`).

## Synopsis

```
tollvm <file.tm>              Compile and print LLVM IR to stdout
tollvm <file.tm> <output>     Compile and emit native binary
tollvm <file.tm> <output> -oN Compile with optimization level N
tollvm helpc <lib> [symbol]   C library reference helper
tollvm help                   Show help message
tollvm version                Show compiler version
```

## Arguments

### Input File (Required)

The first positional argument is the path to a `.tm` source file:

```bash
tollvm main.tm
tollvm examples/hello.tm
tollvm src/app.tm
```

The compiler scans the directory tree containing this file to discover all `.tm` modules (for namespace resolution and multi-file projects).

### Output File (Optional)

The second positional argument is the path for the compiled native binary:

```bash
tollvm main.tm ./main
tollvm src/app.tm build/app
```

If omitted, the compiler prints the generated LLVM IR to stdout instead of producing a binary:

```bash
tollvm main.tm              # prints IR to stdout
tollvm main.tm > output.ll  # redirect IR to a file
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
tollvm main.tm ./main -o2
```

### Linker Flags

```
-lname    Link against library 'name'
```

Links the output binary against a system or third-party library. The flag is passed directly to the system linker.

```bash
tollvm app.tm ./app -lSDL2           # link against SDL2
tollvm app.tm ./app -lm -lpthread    # link against math and pthreads
```

### Library Search Paths

```
-Lpath    Add 'path' to the library search path
```

Adds a directory where the linker looks for libraries specified with `-l`:

```bash
tollvm app.tm ./app -L/opt/lib -lmylib
tollvm app.tm ./app -L./vendor/lib -lvendor
```

### Include Search Paths

```
-Ipath    Add 'path' to the C header include search path
```

Adds a directory where the compiler looks for C header files used with `#include`:

```bash
tollvm app.tm ./app -I/opt/include -lmylib
tollvm app.tm ./app -I./vendor/include -L./vendor/lib -lvendor
```

### Help and Version

```bash
tollvm help        # or --help, -h
tollvm version     # or --version, -v
```

## Examples

### Print LLVM IR (Debugging)

```bash
tollvm main.tm
```

Useful for inspecting the generated IR without producing a binary. The output can be piped or redirected:

```bash
tollvm main.tm | less
tollvm main.tm > debug.ll
```

### Simple Compilation

```bash
tollvm main.tm ./main
./main
```

### Optimized Build

```bash
tollvm main.tm ./main -o2
./main
```

### Linking with External C Libraries

```bash
# Link against SDL2
tollvm game.tm ./game -lSDL2

# Link against a custom library in a non-standard path
tollvm app.tm ./app -L/opt/custom/lib -I/opt/custom/include -lcustom

# Link against multiple libraries
tollvm server.tm ./server -lssl -lcrypto -lpthread
```

### Using Make

The project Makefile provides a shortcut for building and running:

```bash
make run FILE=examples/main.tm
```

## Build Artifacts

When compiling to a binary, the compiler creates a `.tmbuild/` directory in the project root. This directory contains intermediate object files (`.o`) generated during compilation:

```
project/
├── main.tm
├── utils.tm
└── .tmbuild/
    ├── Main__main.o
    ├── Utils__utils.o
    └── c__helper.o      # if local C files are included
```

These files are reused across compilations. You can safely delete `.tmbuild/` to force a clean rebuild.

## Compiler Pipeline

When you run `tollvm main.tm ./main`, the compiler performs these steps in order:

1. **Scan** — Discover all `.tm` files in the project directory
2. **Parse** — Parse each file using the ANTLR4 grammar
3. **Register** — Build a namespace registry for cross-file module resolution
4. **Resolve Headers** — Process `#include` directives (system and local C headers)
5. **Compile C** — Compile any local `.c` files referenced by `#include` to object files
6. **Check** — Run semantic analysis and type checking on all T code
7. **Generate IR** — Emit LLVM intermediate representation
8. **Optimize** — Apply LLVM optimization passes (if `-oN` is specified)
9. **Emit Objects** — Write `.o` object files to `.tmbuild/`
10. **Link** — Invoke the system linker to produce the final native binary

## Error Messages

The compiler produces clear error messages. Some common ones:

```
tollvm: unknown flag '-xyz'
tollvm: Run 'tollvm help' for usage.
```

Unknown command-line flag. Check the flag spelling.

```
tollvm: no .tm files found in '/path/to/project'
```

No T source files found in the directory containing your input file.

```
tollvm: file 'module.tm' is missing a 'namespace' declaration
```

Every `.tm` file must begin with a `namespace` declaration.

```
tollvm: cannot find header '<mylib.h>'. Check '-I' include paths
```

A `#include` directive references a header that cannot be found. Add the correct `-I` path.

## Subcommands

### helpc — C Library Reference

```bash
tollvm helpc <lib> [symbol] [flags]
```

A built-in tool that inspects C library headers and displays detailed information
about functions, structs, enums, macros, and typedefs. Maps every C type to its
T equivalent.

```bash
tollvm helpc raylib InitWindow     # look up a function
tollvm helpc raylib Color          # look up a struct
tollvm helpc raylib --list         # list all symbols
tollvm helpc raylib -s Draw        # search by name
tollvm helpc stdio printf          # works with any installed header
tollvm helpc math sin --json       # JSON output
```

See [helpc](../tools/helpc.md) for the full reference.

## See Also

- [Installation](installation.md) — Build the compiler from source
- [Hello World](hello-world.md) — Your first T program
- [Linking](../ffi/linking.md) — Detailed guide on linking with C libraries
