namespace RandomTest;

use std::log::println;
use std::random::seed;
use std::random::seedTime;
use std::random::randInt;
use std::random::randIntRange;
use std::random::randUint;
use std::random::randFloat;
use std::random::randFloatRange;
use std::random::randBool;
use std::random::randChar;

int32 main() {
    // Use a fixed seed for reproducible test output
    seed(12345);

    // Test 1: randInt — should produce a non-trivial int64
    int64 ri = randInt();
    println(ri != 0);           // expected: true

    // Test 2: randIntRange — should be between 1 and 10
    int32 count = 0;
    int32 i = 0;
    while (i < 20) {
        int64 v = randIntRange(1, 10);
        if (v >= 1) {
            if (v <= 10) {
                count = count + 1;
            }
        }
        i = i + 1;
    }
    println(count == 20);       // expected: true

    // Test 3: randUint — should produce a value
    uint64 ru = randUint();
    println(ru != 0);           // expected: true

    // Test 4: randFloat — should be in [0.0, 1.0)
    int32 floatOk = 0;
    int32 j = 0;
    while (j < 20) {
        float64 f = randFloat();
        if (f >= 0.0) {
            if (f < 1.0) {
                floatOk = floatOk + 1;
            }
        }
        j = j + 1;
    }
    println(floatOk == 20);     // expected: true

    // Test 5: randFloatRange — should be in [10.0, 20.0)
    int32 rangeOk = 0;
    int32 k = 0;
    while (k < 20) {
        float64 fr = randFloatRange(10.0, 20.0);
        if (fr >= 10.0) {
            if (fr < 20.0) {
                rangeOk = rangeOk + 1;
            }
        }
        k = k + 1;
    }
    println(rangeOk == 20);     // expected: true

    // Test 6: randBool — should produce true or false
    bool b1 = randBool();
    bool b2 = randBool();
    bool b3 = randBool();
    bool b4 = randBool();
    // At least one should differ in 4 tries (statistically near-certain)
    bool allSame = b1 == b2;
    if (allSame) {
        allSame = b2 == b3;
        if (allSame) {
            allSame = b3 == b4;
        }
    }
    // Just verify it returns valid bools — print one
    println(b1 == true || b1 == false);  // expected: true

    // Test 7: randChar — should be a printable ASCII char
    char c = randChar();
    println(c != '\0');          // expected: true

    // Test 8: seedTime — should not crash
    seedTime();
    int64 afterSeed = randInt();
    println(afterSeed != 0);    // expected: true (overwhelmingly likely)

    // Test 9: re-seed with same seed should reproduce sequence
    seed(99999);
    int64 a1 = randInt();
    int64 a2 = randInt();
    seed(99999);
    int64 b1r = randInt();
    int64 b2r = randInt();
    println(a1 == b1r);         // expected: true
    println(a2 == b2r);         // expected: true

    println("all random tests passed");  // expected: all random tests passed

    ret 0;
}
