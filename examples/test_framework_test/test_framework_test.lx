namespace TestFrameworkTest;

use std::test::assertEqual;
use std::test::assertNotEqual;
use std::test::assertTrue;
use std::test::assertFalse;
use std::test::assertGreater;
use std::test::assertLess;
use std::test::assertGreaterEq;
use std::test::assertLessEq;
use std::test::assertStringContains;
use std::test::assertNear;
use std::test::log;
use std::log::println;

int32 main() {
    // ── assertEqual ────────────────────────────────────────────
    assertEqual(42, 42);
    assertEqual(0, 0);
    assertEqual(-100, -100);
    log("assertEqual int: OK");

    assertEqual(3.14, 3.14);
    assertEqual(0.0, 0.0);
    log("assertEqual float: OK");

    assertEqual("hello", "hello");
    assertEqual("", "");
    log("assertEqual string: OK");

    assertEqual(true, true);
    assertEqual(false, false);
    log("assertEqual bool: OK");

    assertEqual('a', 'a');
    assertEqual('Z', 'Z');
    log("assertEqual char: OK");

    // ── assertNotEqual ─────────────────────────────────────────
    assertNotEqual(1, 2);
    assertNotEqual(-5, 5);
    log("assertNotEqual int: OK");

    assertNotEqual(1.0, 2.0);
    log("assertNotEqual float: OK");

    assertNotEqual("foo", "bar");
    assertNotEqual("", "x");
    log("assertNotEqual string: OK");

    assertNotEqual(true, false);
    log("assertNotEqual bool: OK");

    assertNotEqual('a', 'b');
    log("assertNotEqual char: OK");

    // ── assertTrue / assertFalse ───────────────────────────────
    assertTrue(true);
    assertTrue(1 == 1);
    assertFalse(false);
    assertFalse(1 == 2);
    log("assertTrue/assertFalse: OK");

    // ── assertGreater ──────────────────────────────────────────
    assertGreater(10, 5);
    assertGreater(-1, -10);
    assertGreater(3.14, 2.71);
    log("assertGreater: OK");

    // ── assertLess ─────────────────────────────────────────────
    assertLess(5, 10);
    assertLess(-10, -1);
    assertLess(2.71, 3.14);
    log("assertLess: OK");

    // ── assertGreaterEq ────────────────────────────────────────
    assertGreaterEq(10, 10);
    assertGreaterEq(11, 10);
    assertGreaterEq(3.14, 3.14);
    assertGreaterEq(3.15, 3.14);
    log("assertGreaterEq: OK");

    // ── assertLessEq ───────────────────────────────────────────
    assertLessEq(10, 10);
    assertLessEq(9, 10);
    assertLessEq(3.14, 3.14);
    assertLessEq(3.13, 3.14);
    log("assertLessEq: OK");

    // ── assertStringContains ───────────────────────────────────
    assertStringContains("hello world", "world");
    assertStringContains("hello world", "hello");
    assertStringContains("abc", "");
    log("assertStringContains: OK");

    // ── assertNear ─────────────────────────────────────────────
    assertNear(3.14159, 3.14160, 0.001);
    assertNear(1.0, 1.0, 0.0);
    assertNear(0.1 + 0.2, 0.3, 0.0001);
    log("assertNear: OK");

    // ── log ────────────────────────────────────────────────────
    log("all 13 std::test functions verified");

    println("all assertions passed");

    ret 0;
}
