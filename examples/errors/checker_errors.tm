namespace CheckerErrors;

use std::log::println;

struct Point {
    int32 x;
    int32 y;
}

enum Color { Red, Green, Blue }

int32 main() {
    // ERROR 1: undefined variable
    x = 10;

    // ERROR 2: arithmetic on non-numeric type
    bool flag = true;
    int32 bad1 = flag + 1;

    // ERROR 3: bitwise on float
    float64 f = 3.14;
    int32 bad2 = f & 1;

    // ERROR 4: modulo on float
    float64 bad3 = f % 2.0;

    // ERROR 5: increment on non-integer
    f++;

    // ERROR 6: undefined variable in expression
    int32 bad4 = unknown_var + 1;

    // ERROR 7: duplicate variable
    int32 flag = 42;

    // ERROR 8: wrong struct field name
    Point p = Point { x: 1, z: 2 };

    // ERROR 9: unknown enum variant
    Color c = Color::Yellow;

    // ERROR 10: dereference non-pointer
    int32 val = 42;
    *val = 10;

    // ERROR 11: shift on float
    float64 bad5 = f << 2;

    // ERROR 12: compound assign with wrong type
    bool b = true;
    b += 1;

    // ERROR 13: relational comparison on non-numeric
    bool bad6 = flag > b;

    ret 0;
}
