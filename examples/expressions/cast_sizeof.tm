namespace CastSizeofTest;

use std::log::println;

int32 main() {
    // Cast int to wider int
    int32 a = 42;
    int64 b = a as int64;
    println(b);

    // Cast int to narrower int
    int64 c = 1000;
    int32 d = c as int32;
    println(d);

    // Cast int to float
    int32 x = 7;
    float64 f = x as float64;
    println(f);

    // Cast float to int (truncation)
    float64 pi = 3.14;
    int32 ipi = pi as int32;
    println(ipi);

    // sizeof primitive types
    int64 s1 = sizeof(int8);
    println(s1);

    int64 s2 = sizeof(int32);
    println(s2);

    int64 s3 = sizeof(int64);
    println(s3);

    int64 s4 = sizeof(float64);
    println(s4);

    int64 s5 = sizeof(bool);
    println(s5);

    int64 s6 = sizeof(char);
    println(s6);

    ret 0;
}
