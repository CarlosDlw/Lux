namespace SetTest;

use std::log::println;
use std::collections::Set;

int32 main() {
    // ── Set<int32> ────────────────────────────────────────────────────
    Set<int32> si = [];

    // isEmpty on new set
    bool empty = si.isEmpty();
    println(empty);      // expect: true

    // add elements
    bool added1 = si.add(10);
    bool added2 = si.add(20);
    bool added3 = si.add(30);
    println(added1);     // expect: true
    println(added2);     // expect: true
    println(added3);     // expect: true

    // len after adding 3
    usize slen = si.len();
    println(slen);       // expect: 3

    // isEmpty after adding
    bool empty2 = si.isEmpty();
    println(empty2);     // expect: false

    // add duplicate
    bool dup = si.add(20);
    println(dup);        // expect: false

    // len unchanged
    usize slen2 = si.len();
    println(slen2);      // expect: 3

    // has
    bool h1 = si.has(10);
    bool h2 = si.has(99);
    println(h1);         // expect: true
    println(h2);         // expect: false

    // remove existing
    bool r1 = si.remove(20);
    println(r1);         // expect: true

    // remove non-existing
    bool r2 = si.remove(20);
    println(r2);         // expect: false

    // len after remove
    usize slen3 = si.len();
    println(slen3);      // expect: 2

    // has after remove
    bool h3 = si.has(20);
    println(h3);         // expect: false

    // clear
    si.clear();
    usize slen4 = si.len();
    println(slen4);      // expect: 0

    // free
    si.free();

    // ── Set<string> ──────────────────────────────────────────────────
    Set<string> ss = [];

    bool sa1 = ss.add("hello");
    bool sa2 = ss.add("world");
    bool sa3 = ss.add("hello");
    println(sa1);        // expect: true
    println(sa2);        // expect: true
    println(sa3);        // expect: false

    usize sslen = ss.len();
    println(sslen);      // expect: 2

    bool sh1 = ss.has("hello");
    bool sh2 = ss.has("foo");
    println(sh1);        // expect: true
    println(sh2);        // expect: false

    bool sr1 = ss.remove("world");
    println(sr1);        // expect: true

    usize sslen2 = ss.len();
    println(sslen2);     // expect: 1

    ss.free();

    // ── Set<uint64> ─────────────────────────────────────────────────
    Set<uint64> su = [];

    su.add(100);
    su.add(200);
    su.add(300);

    bool suh = su.has(200);
    println(suh);        // expect: true

    usize sulen = su.len();
    println(sulen);      // expect: 3

    su.free();

    ret 0;
}
