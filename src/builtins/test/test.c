#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

/* ── Internal helpers ───────────────────────────────────────────── */

static void fail_abort(const char* type, const char* detail) {
    fprintf(stderr, "\033[31mASSERTION FAILED\033[0m [%s]", type);
    if (detail) fprintf(stderr, ": %s", detail);
    fprintf(stderr, "\n");
    abort();
}

/* ── assertEqual ────────────────────────────────────────────────── */

void tollvm_assertEqualI64(int64_t a, int64_t b) {
    if (a != b) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "expected %" PRId64 ", got %" PRId64, b, a);
        fail_abort("assertEqual", buf);
    }
}

void tollvm_assertEqualF64(double a, double b) {
    if (a != b) {
        char buf[128];
        snprintf(buf, sizeof(buf), "expected %g, got %g", b, a);
        fail_abort("assertEqual", buf);
    }
}

void tollvm_assertEqualStr(const char* a, size_t alen,
                           const char* b, size_t blen) {
    if (alen != blen || memcmp(a, b, alen) != 0) {
        char buf[512];
        snprintf(buf, sizeof(buf), "expected \"%.*s\", got \"%.*s\"",
                 (int)(blen > 200 ? 200 : blen), b,
                 (int)(alen > 200 ? 200 : alen), a);
        fail_abort("assertEqual", buf);
    }
}

void tollvm_assertEqualBool(int32_t a, int32_t b) {
    if ((!a) != (!b)) {
        char buf[64];
        snprintf(buf, sizeof(buf), "expected %s, got %s",
                 b ? "true" : "false", a ? "true" : "false");
        fail_abort("assertEqual", buf);
    }
}

void tollvm_assertEqualChar(int8_t a, int8_t b) {
    if (a != b) {
        char buf[64];
        snprintf(buf, sizeof(buf), "expected '%c' (%d), got '%c' (%d)",
                 (char)b, (int)b, (char)a, (int)a);
        fail_abort("assertEqual", buf);
    }
}

/* ── assertNotEqual ─────────────────────────────────────────────── */

void tollvm_assertNotEqualI64(int64_t a, int64_t b) {
    if (a == b) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "values should differ, both are %" PRId64, a);
        fail_abort("assertNotEqual", buf);
    }
}

void tollvm_assertNotEqualF64(double a, double b) {
    if (a == b) {
        char buf[128];
        snprintf(buf, sizeof(buf), "values should differ, both are %g", a);
        fail_abort("assertNotEqual", buf);
    }
}

void tollvm_assertNotEqualStr(const char* a, size_t alen,
                              const char* b, size_t blen) {
    if (alen == blen && memcmp(a, b, alen) == 0) {
        char buf[512];
        snprintf(buf, sizeof(buf), "values should differ, both are \"%.*s\"",
                 (int)(alen > 200 ? 200 : alen), a);
        fail_abort("assertNotEqual", buf);
    }
}

void tollvm_assertNotEqualBool(int32_t a, int32_t b) {
    if ((!a) == (!b)) {
        char buf[64];
        snprintf(buf, sizeof(buf), "values should differ, both are %s",
                 a ? "true" : "false");
        fail_abort("assertNotEqual", buf);
    }
}

void tollvm_assertNotEqualChar(int8_t a, int8_t b) {
    if (a == b) {
        char buf[64];
        snprintf(buf, sizeof(buf),
                 "values should differ, both are '%c' (%d)",
                 (char)a, (int)a);
        fail_abort("assertNotEqual", buf);
    }
}

/* ── assertTrue / assertFalse ───────────────────────────────────── */

void tollvm_assertTrue(int32_t cond) {
    if (!cond) fail_abort("assertTrue", "expected true, got false");
}

void tollvm_assertFalse(int32_t cond) {
    if (cond) fail_abort("assertFalse", "expected false, got true");
}

/* ── assertGreater ──────────────────────────────────────────────── */

void tollvm_assertGreaterI64(int64_t a, int64_t b) {
    if (!(a > b)) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "expected %" PRId64 " > %" PRId64, a, b);
        fail_abort("assertGreater", buf);
    }
}

void tollvm_assertGreaterF64(double a, double b) {
    if (!(a > b)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "expected %g > %g", a, b);
        fail_abort("assertGreater", buf);
    }
}

/* ── assertLess ─────────────────────────────────────────────────── */

void tollvm_assertLessI64(int64_t a, int64_t b) {
    if (!(a < b)) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "expected %" PRId64 " < %" PRId64, a, b);
        fail_abort("assertLess", buf);
    }
}

void tollvm_assertLessF64(double a, double b) {
    if (!(a < b)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "expected %g < %g", a, b);
        fail_abort("assertLess", buf);
    }
}

/* ── assertGreaterEq ────────────────────────────────────────────── */

void tollvm_assertGreaterEqI64(int64_t a, int64_t b) {
    if (!(a >= b)) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "expected %" PRId64 " >= %" PRId64, a, b);
        fail_abort("assertGreaterEq", buf);
    }
}

void tollvm_assertGreaterEqF64(double a, double b) {
    if (!(a >= b)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "expected %g >= %g", a, b);
        fail_abort("assertGreaterEq", buf);
    }
}

/* ── assertLessEq ───────────────────────────────────────────────── */

void tollvm_assertLessEqI64(int64_t a, int64_t b) {
    if (!(a <= b)) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "expected %" PRId64 " <= %" PRId64, a, b);
        fail_abort("assertLessEq", buf);
    }
}

void tollvm_assertLessEqF64(double a, double b) {
    if (!(a <= b)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "expected %g <= %g", a, b);
        fail_abort("assertLessEq", buf);
    }
}

/* ── assertStringContains ───────────────────────────────────────── */

void tollvm_assertStringContains(const char* s, size_t slen,
                                 const char* sub, size_t sublen) {
    if (sublen == 0) return; /* empty substring always contained */
    if (sublen > slen) {
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "\"%.*s\" does not contain \"%.*s\"",
                 (int)(slen > 200 ? 200 : slen), s,
                 (int)(sublen > 200 ? 200 : sublen), sub);
        fail_abort("assertStringContains", buf);
    }
    for (size_t i = 0; i <= slen - sublen; i++) {
        if (memcmp(s + i, sub, sublen) == 0) return;
    }
    char buf[512];
    snprintf(buf, sizeof(buf),
             "\"%.*s\" does not contain \"%.*s\"",
             (int)(slen > 200 ? 200 : slen), s,
             (int)(sublen > 200 ? 200 : sublen), sub);
    fail_abort("assertStringContains", buf);
}

/* ── assertNear ─────────────────────────────────────────────────── */

void tollvm_assertNear(double a, double b, double epsilon) {
    if (fabs(a - b) > epsilon) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "|%g - %g| = %g > epsilon %g",
                 a, b, fabs(a - b), epsilon);
        fail_abort("assertNear", buf);
    }
}

/* ── Utilities ──────────────────────────────────────────────────── */

void tollvm_testFail(const char* msg, size_t msglen) {
    char buf[512];
    snprintf(buf, sizeof(buf), "%.*s", (int)(msglen > 500 ? 500 : msglen), msg);
    fail_abort("fail", buf);
}

void tollvm_testSkip(const char* msg, size_t msglen) {
    fprintf(stderr, "\033[33mSKIP\033[0m: %.*s\n",
            (int)(msglen > 500 ? 500 : msglen), msg);
    exit(0);
}

void tollvm_testLog(const char* msg, size_t msglen) {
    fprintf(stderr, "\033[36mLOG\033[0m: %.*s\n",
            (int)(msglen > 500 ? 500 : msglen), msg);
}
