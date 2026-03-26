namespace MemTest;

use std::log::println;
use std::mem::alloc;
use std::mem::allocZeroed;
use std::mem::realloc;
use std::mem::free;
use std::mem::copy;
use std::mem::move;
use std::mem::set;
use std::mem::zero;
use std::mem::compare;

int32 main() {
    // Test 1: alloc + set + dereference
    *int32 p = alloc(4);
    *p = 42;
    println(*p);            // expected: 42

    // Test 2: allocZeroed — memory should be zero
    *int32 q = allocZeroed(4);
    println(*q);            // expected: 0

    // Test 3: copy — copy value from p to q
    copy(q, p, 4);
    println(*q);            // expected: 42

    // Test 4: compare — equal memory
    int32 cmp1 = compare(p, q, 4);
    println(cmp1);          // expected: 0

    // Test 5: set — fill with a byte value
    *q = 0;
    set(q, 255, 4);
    // 0xFFFFFFFF as signed int32 = -1
    println(*q);            // expected: -1

    // Test 6: zero — zero out memory
    zero(q, 4);
    println(*q);            // expected: 0

    // Test 7: compare — different memory
    *p = 100;
    *q = 200;
    int32 cmp2 = compare(p, q, 4);
    // 100 < 200, so memcmp returns negative
    println(cmp2 < 0);      // expected: true

    // Test 8: move — overlapping-safe copy
    *int32 buf = alloc(16);
    *int32 buf0 = buf;
    *buf0 = 10;
    // Use pointer arithmetic via second alloc to test move
    *int32 src = alloc(4);
    *src = 77;
    move(buf, src, 4);
    println(*buf);          // expected: 77

    // Test 9: realloc — resize allocation
    *int32 r = alloc(4);
    *r = 55;
    r = realloc(r, 8);
    println(*r);            // expected: 55

    // Test 10: free — no crash
    free(p);
    free(q);
    free(buf);
    free(src);
    free(r);
    println("all tests passed");  // expected: all tests passed

    ret 0;
}
