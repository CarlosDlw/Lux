namespace BitwiseTest;

use std::log::println;

int32 main() {
    int32 a = 12;
    int32 b = 10;

    // Bitwise AND
    int32 band = a & b;
    println(band);

    // Bitwise OR
    int32 bor = a | b;
    println(bor);

    // Bitwise XOR
    int32 bxor = a ^ b;
    println(bxor);

    // Bitwise NOT
    int32 bnot = ~0;
    println(bnot);

    // Left shift
    int32 shl = 1 << 4;
    println(shl);

    // Right shift
    int32 shr = 64 >> 2;
    println(shr);

    // Combined: set bit 3 and check
    int32 flags = 0 | (1 << 3);
    println(flags);

    int32 mask = flags & 8;
    println(mask);

    // XOR swap trick
    int32 x = 42;
    int32 y = 99;
    int32 swapped = x ^ y;
    println(swapped);

    ret 0;
}
