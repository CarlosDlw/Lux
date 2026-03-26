namespace uninit_test;

use std::collections::{ Vec, Map, Set };

extern void printf(cstring fmt, ...);

void main() {
    // Scalar types — should be zero-initialized
    int32 x;
    int64 y;
    float64 z;
    bool flag;

    printf(c"int32: %d\n", x);
    printf(c"int64: %ld\n", y);
    printf(c"float64: %f\n", z);
    printf(c"bool: %d\n", flag);

    // Now assign and print
    x = 42;
    y = 100;
    z = 3.14;
    flag = true;

    printf(c"int32 after assign: %d\n", x);
    printf(c"int64 after assign: %ld\n", y);
    printf(c"float64 after assign: %f\n", z);
    printf(c"bool after assign: %d\n", flag);

    // Vec — should be empty-initialized
    Vec<int32> v;
    printf(c"vec size (should be 0): done\n");

    // Fixed-size array — should be zero-initialized
    [3]int32 arr;
    printf(c"arr[0]: %d\n", arr[0]);
    printf(c"arr[1]: %d\n", arr[1]);
    printf(c"arr[2]: %d\n", arr[2]);

    Map<string, int32> m;
    Set<string> s;
    char c;
    string st;
}
