namespace SwitchTest;

use std::log::println;

int32 main() {
    // Test 1: basic switch without parens
    int32 x = 2;
    switch x {
        case 1 {
            println(1);
        }
        case 2 {
            println(2);
        }
        case 3 {
            println(3);
        }
        default {
            println(0);
        }
    }

    // Test 2: switch with parens
    int32 y = 10;
    switch (y) {
        case 10 {
            println(10);
        }
        default {
            println(99);
        }
    }

    // Test 3: multi-value case
    int32 z = 5;
    switch z {
        case 1, 2, 3 {
            println(100);
        }
        case 4, 5, 6 {
            println(200);
        }
        default {
            println(300);
        }
    }

    // Test 4: default fallback
    int32 w = 42;
    switch w {
        case 1 {
            println(1);
        }
        default {
            println(42);
        }
    }

    // Test 5: switch without default
    int32 v = 3;
    switch v {
        case 1 {
            println(1);
        }
        case 3 {
            println(3);
        }
    }

    ret 0;
}
