# Enums

Enums define a type with a fixed set of named variants. Each variant is represented as a sequential integer starting from 0.

---

## Declaration

An enum is defined with the `enum` keyword followed by a name and a block of comma-separated variants:

```tm
enum Color {
    Red,
    Green,
    Blue,
}
```

```tm
enum Direction {
    North,
    South,
    East,
    West,
}
```

Variants are automatically assigned integer values starting from 0: `Red = 0`, `Green = 1`, `Blue = 2`.

---

## Using Enum Values

Enum variants are accessed with the `::` (scope resolution) operator:

```tm
Color c = Color::Red;
println(c);   // 0

Color g = Color::Green;
println(g);   // 1

Color b = Color::Blue;
println(b);   // 2
```

---

## Enums in Switch

Enums work naturally with `switch` statements:

```tm
Color c = Color::Green;

switch c {
    case Color::Red {
        println("red");
    }
    case Color::Green {
        println("green");
    }
    case Color::Blue {
        println("blue");
    }
}
```

---

## Enums in Structs

Enums can be used as struct fields:

```tm
enum Color {
    Red,
    Green,
    Blue,
}

struct Particle {
    Vec2 pos;
    int32 speed;
    Color color;
    bool active;
}

Particle p = Particle { pos: pos, speed: 1, color: Color::Red, active: true };
```

---

## Enum Methods with `extend`

Like structs, enums can have methods added via `extend` blocks:

```tm
enum Direction {
    North,
    South,
    East,
    West,
}

extend Direction {
    bool isVertical(&self) {
        ret *self == Direction::North || *self == Direction::South;
    }
}
```

---

## C Enum Interop

C enums from `#include` headers are automatically imported and mapped to integer constants:

```c
// colors.h
typedef enum {
    COLOR_RED   = 0,
    COLOR_GREEN = 1,
    COLOR_BLUE  = 2
} Color;

const char* color_name(Color c);
Color next_color(Color c);
```

```tm
#include "colors.h"

int32 r = COLOR_RED;     // 0
int32 g = COLOR_GREEN;   // 1
int32 b = COLOR_BLUE;    // 2

*char name = color_name(COLOR_GREEN);
int32 next = next_color(COLOR_BLUE);   // 0 (wraps to RED)
```

C enum variants are imported as `int32` constants with their original names.

---

## See Also

- [Structs](structs.md) — Composite types with named fields
- [Unions](unions.md) — Types with shared memory layout
- [Control Flow](control-flow.md) — `switch` statements
