namespace HashTest;

use std::hash::hashString;
use std::hash::hashInt;
use std::hash::hashCombine;
use std::log::println;

int32 main() {
    /* hashString — deterministic FNV-1a */
    uint64 h1 = hashString("hello");
    println(h1);

    uint64 h2 = hashString("hello");
    println(h1 == h2);

    uint64 h3 = hashString("world");
    println(h1 != h3);

    uint64 h4 = hashString("");
    println(h4);

    /* hashInt — deterministic splitmix64 */
    uint64 hi1 = hashInt(42);
    println(hi1);

    uint64 hi2 = hashInt(42);
    println(hi1 == hi2);

    uint64 hi3 = hashInt(0);
    println(hi3);

    uint64 hi4 = hashInt(-1);
    println(hi4);

    /* hashCombine — deterministic combine */
    uint64 c1 = hashCombine(h1, h3);
    println(c1);

    uint64 c2 = hashCombine(h1, h3);
    println(c1 == c2);

    /* combine is order-dependent */
    uint64 c3 = hashCombine(h3, h1);
    println(c1 != c3);

    println("all hash tests passed");
    ret 0;
}
