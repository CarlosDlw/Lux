namespace DeferTest;

use std::log::println;
use std::collections::{ Vec, Map, Set };

#include <stdlib.h>

void test_defer() {
    *int32 ptr = malloc(4) as *int32;
    defer free(ptr as *void);

    *ptr = 42;
    println(*ptr);
}

void test_defer_order() {
    defer println(1);
    defer println(2);
    defer println(3);
    println(0);
}

void test_vec_auto() {
    Vec<int32> v;
    v.push(10);
    v.push(20);
    v.push(30);
    println(v.len());
}

void test_map_auto() {
    Map<int32, int32> m;
    m.set(1, 100);
    m.set(2, 200);
    println(m.len());
}

void test_set_auto() {
    Set<int32> s;
    s.add(1);
    s.add(2);
    s.add(3);
    println(s.len());
}

void test_mixed() {
    Vec<int32> v;
    v.push(1);
    defer println(99);
    Vec<int32> w;
    w.push(2);
    w.push(3);
    println(v.len());
    println(w.len());
}

int32 main() {
    println("=== defer manual ===");
    test_defer();

    println("=== defer LIFO order ===");
    test_defer_order();

    println("=== Vec auto-cleanup ===");
    test_vec_auto();

    println("=== Map auto-cleanup ===");
    test_map_auto();

    println("=== Set auto-cleanup ===");
    test_set_auto();

    println("=== mixed defer + auto ===");
    test_mixed();

    ret 0;
}
