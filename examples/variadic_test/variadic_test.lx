namespace VariadicTest;

use std::log::println;

int32 add(int32 ...n) {
    int32 sum = 0;
    for int32 x in n {
        sum = sum + x;
    }
    ret sum;
}

void printAll(int32 ...n) {
    for int32 x in n {
        println(x);
    }
}

int32 addWithBase(int32 base, int32 ...n) {
    int32 sum = base;
    for int32 x in n {
        sum = sum + x;
    }
    ret sum;
}

int32 first(int32 ...n) {
    ret n[0];
}

int32 count(int32 ...n) {
    int64 len = n.len;
    ret len;
}

int32 main() {
    int32 result = add(1, 2, 3, 4, 5);
    println(result);

    int32 r2 = add(10, 20);
    println(r2);

    int32 r3 = addWithBase(100, 1, 2, 3);
    println(r3);

    printAll(10, 20, 30);

    int32 r4 = add();
    println(r4);

    int32 f = first(42, 99);
    println(f);

    int32 c = count(1, 2, 3, 4);
    println(c);

    ret 0;
}
