namespace CInteropTest;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int32 main() {
    printf(c"=== C Interop Full Test Suite ===\n");

    // --- Test 1: malloc + free (basic pointer ops) ---
    *int32 ptr = malloc(4) as *int32;
    *ptr = 42;
    printf(c"[PASS] malloc + deref assign: %d\n", *ptr);
    free(ptr as *void);

    // --- Test 2: pointer arithmetic ---
    *int32 buf = malloc(20) as *int32;
    *buf = 10;
    *(buf + 1) = 20;
    *(buf + 2) = 30;
    printf(c"[PASS] ptr arith write: %d, %d, %d\n", *buf, *(buf + 1), *(buf + 2));
    // Also test with intermediate variable
    *int32 p4 = buf + 3;
    *p4 = 40;
    printf(c"[PASS] ptr arith var: %d\n", *p4);
    free(buf as *void);

    // --- Test 3: strlen ---
    usize len = strlen(c"Hello, World!");
    printf(c"[PASS] strlen: %lu\n", len);

    // --- Test 4: memcpy ---
    *void dst = malloc(10);
    memcpy(dst, c"TestData!" as *void, 9);
    printf(c"[PASS] memcpy result: %.9s\n", dst as *char);
    free(dst);

    // --- Test 5: strcmp ---
    int32 cmp = strcmp(c"abc", c"abc");
    printf(c"[PASS] strcmp equal: %d\n", cmp);

    printf(c"=== All C interop tests passed ===\n");
    ret 0;
}
