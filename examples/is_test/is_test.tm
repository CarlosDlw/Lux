namespace IsTest;

use std::log::println;

int32 main() {
    int32 x = 42;
    bool check = x is int32;
    println(check);

    bool check2 = x is int64;
    println(check2);

    ret 0;
}
