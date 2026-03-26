namespace UintStd;

/*
    unsigned integer types: uint1, uint8, uint16, uint32, uint64, uint128, usize.
    same as int types but unsigned — no negative values.
*/
use std::log::println;

int32 main() {
    uint1 a = 1;        // 1-bit unsigned: 0 or 1
    uint8 b = 200;      // 8-bit unsigned: 0 to 255
    uint16 c = 50000;   // 16-bit unsigned: 0 to 65535
    uint32 d = 3000000; // 32-bit unsigned: 0 to 4294967295
    uint64 e = 10000000000; // 64-bit unsigned: 0 to 18446744073709551615
    uint128 f = 100000000000000000; // 128-bit unsigned
    usize g = 10000000; // pointer-sized unsigned integer

    println(a);
    println(b);
    println(c);
    println(d);
    println(e);
    println(f);
    println(g);

    ret 0;
}
