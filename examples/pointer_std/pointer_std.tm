namespace PointerStdTest;

use std::log::println;

int32 main() {
    int32 x = 42;
    *int32 p = &x;

    // dereference: read value through pointer
    println(*p);

    // dereference assign: write through pointer
    *p = 99;
    println(x);

    // null pointer
    *int32 n = null;

    ret 0;
}
