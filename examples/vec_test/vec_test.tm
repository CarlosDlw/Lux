namespace VecTest;

use std::log::println;
use std::collections::Vec;

int32 main() {
    // Create a Vec from array literal
    Vec<int32> v = [10, 20, 30];

    // Test len
    int64 length = v.len();
    println(length);

    // Test element access
    int32 first = v.first();
    println(first);

    int32 last = v.last();
    println(last);

    int32 mid = v.at(1);
    println(mid);

    // Test push
    v.push(40);
    int64 newLen = v.len();
    println(newLen);

    // Test pop
    int32 popped = v.pop();
    println(popped);

    // Test isEmpty
    bool empty = v.isEmpty();
    println(empty);

    // Test contains
    bool has20 = v.contains(20);
    println(has20);

    bool has99 = v.contains(99);
    println(has99);

    // Test empty Vec
    Vec<int32> empty_v = [];
    int64 emptyLen = empty_v.len();
    println(emptyLen);

    // Cleanup
    v.free();
    empty_v.free();

    // Test with different types
    Vec<float64> fv = [1.5, 2.5, 3.5];
    float64 fsum = fv.first();
    println(fsum);

    fv.push(4.5);
    float64 flast = fv.last();
    println(flast);

    fv.free();

    // Test index access and assignment
    Vec<int32> iv = [100, 200, 300];

    // Index read
    int32 val0 = iv[0];
    println(val0);

    int32 val2 = iv[2];
    println(val2);

    // Index write
    iv[1] = 999;
    int32 updated = iv[1];
    println(updated);

    iv.free();

    ret 0;
}
