namespace GlobalBuiltins;

use std::log::println;

int32 main() {
    // ── toString ────────────────────────────────
    string s1 = toString(42);
    println(s1);

    string s2 = toString(3.14);
    println(s2);

    string s3 = toString(true);
    println(s3);

    string s4 = toString('A');
    println(s4);

    // ── toInt / toFloat / toBool ────────────────
    int64 n = toInt("123");
    println(n);

    float64 f = toFloat("2.718");
    println(f);

    bool b = toBool("true");
    println(b);

    // ── assert (should pass) ────────────────────
    assert(true);
    assertMsg(true, "this should pass");

    // ── exit ────────────────────────────────────
    exit(0);

    ret 0;
}
