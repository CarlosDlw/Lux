namespace NullCoalTest;

use std::log::println;

int32 main() {
    int32 val = 42;
    *int32 ptr = &val;

    int32 result = ptr ?? 0;
    println(result);

    *int32 nilPtr = null;
    int32 fallback = nilPtr ?? 99;
    println(fallback);

    ret 0;
}
