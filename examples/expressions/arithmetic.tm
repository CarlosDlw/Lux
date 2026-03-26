namespace ArithmeticTest;

use std::log::println;

int32 main() {
    // Basic arithmetic with literals
    int32 a = 10 + 5;
    println(a);

    int32 b = 20 - 8;
    println(b);

    int32 c = 6 * 7;
    println(c);

    int32 d = 100 / 4;
    println(d);

    int32 e = 17 % 5;
    println(e);

    // Arithmetic with variables
    int32 x = 10;
    int32 y = 3;
    int32 sum = x + y;
    println(sum);

    int32 diff = x - y;
    println(diff);

    int32 prod = x * y;
    println(prod);

    int32 quot = x / y;
    println(quot);

    int32 rem = x % y;
    println(rem);

    // Unary negation
    int32 neg = -42;
    println(neg);

    // Parenthesized expressions / precedence
    int32 p1 = 2 + 3 * 4;
    println(p1);

    int32 p2 = (2 + 3) * 4;
    println(p2);

    int32 p3 = 100 / (5 + 5);
    println(p3);

    // Nested operations
    int32 nested = (10 + 5) * (20 - 8) / 3;
    println(nested);

    ret 0;
}
