namespace MapTest;

use std::log::println;
use std::collections::Map;

int32 main() {
    // ── Map<string, int64> ──────────────────────────────────────────
    Map<string, int64> ages = [];

    // isEmpty on empty map
    bool empty = ages.isEmpty();
    println(empty);

    // set and len
    ages.set("alice", 30);
    ages.set("bob", 25);
    ages.set("carol", 35);
    int64 count = ages.len();
    println(count);

    // get by key
    int64 aliceAge = ages.get("alice");
    println(aliceAge);

    int64 bobAge = ages.get("bob");
    println(bobAge);

    // has
    bool hasAlice = ages.has("alice");
    println(hasAlice);

    bool hasDave = ages.has("dave");
    println(hasDave);

    // getOrDefault
    int64 daveAge = ages.getOrDefault("dave", 0);
    println(daveAge);

    int64 carolAge = ages.getOrDefault("carol", 0);
    println(carolAge);

    // update existing key
    ages.set("alice", 31);
    int64 newAlice = ages.get("alice");
    println(newAlice);

    // remove
    bool removed = ages.remove("bob");
    println(removed);

    bool removeMissing = ages.remove("nobody");
    println(removeMissing);

    int64 afterRemove = ages.len();
    println(afterRemove);

    // isEmpty after operations
    bool notEmpty = ages.isEmpty();
    println(notEmpty);

    // subscript read: m[key]
    int64 sub = ages["carol"];
    println(sub);

    // subscript write: m[key] = val
    ages["dave"] = 40;
    int64 daveVal = ages.get("dave");
    println(daveVal);

    // clear
    ages.clear();
    int64 cleared = ages.len();
    println(cleared);

    bool emptyAfterClear = ages.isEmpty();
    println(emptyAfterClear);

    // free
    ages.free();

    // ── Map<int32, int64> ───────────────────────────────────────────
    Map<int32, int64> scores = [];

    scores.set(1, 100);
    scores.set(2, 200);
    scores.set(3, 300);

    int64 s1 = scores.get(1);
    println(s1);

    int64 s2 = scores.get(2);
    println(s2);

    bool hasKey3 = scores.has(3);
    println(hasKey3);

    int64 s4 = scores.getOrDefault(4, -1);
    println(s4);

    scores.free();

    // ── Map<string, string> ─────────────────────────────────────────
    Map<string, string> dict = [];

    dict.set("hello", "world");
    dict.set("foo", "bar");

    string val1 = dict.get("hello");
    println(val1);

    string val2 = dict.get("foo");
    println(val2);

    bool hasHello = dict.has("hello");
    println(hasHello);

    dict.remove("foo");
    bool hasFoo = dict.has("foo");
    println(hasFoo);

    dict.free();

    ret 0;
}
