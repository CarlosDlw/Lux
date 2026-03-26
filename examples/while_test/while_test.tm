namespace WhileTest;

use std::log::println;

int32 main() {
    // while without parens
    int32 i = 0;
    while i < 5 {
        println(i);
        i += 1;
    }

    // while with parens
    int32 j = 10;
    while (j > 7) {
        println(j);
        j -= 1;
    }

    // do-while without parens (body runs at least once)
    int32 k = 100;
    do {
        println(k);
        k += 1;
    } while k < 103;

    // do-while with parens
    int32 m = 0;
    do {
        println(m);
        m += 10;
    } while (m < 30);

    // while with break
    int32 n = 0;
    while n < 100 {
        if n == 3 {
            break;
        }
        println(n);
        n += 1;
    }

    // while with continue
    int32 p = 0;
    while p < 6 {
        p += 1;
        if p == 2 {
            continue;
        }
        if p == 4 {
            continue;
        }
        println(p);
    }

    ret 0;
}
