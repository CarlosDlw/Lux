namespace IfTest;

use std::log::println;

int32 main() {
    int32 x = 10;

    // with parentheses
    if (x == 10) {
        println(1);
    }

    // without parentheses
    if x > 5 {
        println(100);
    } else {
        println(200);
    }

    // mixed: no parens on if, parens on else-if
    if x == 1 {
        println(1);
    } else if (x == 5) {
        println(5);
    } else if x == 10 {
        println(10);
    } else {
        println(0);
    }

    ret 0;
}
