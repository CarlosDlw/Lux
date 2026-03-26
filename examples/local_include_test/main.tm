namespace LocalIncludeTest;

#include <stdio.h>
#include "mymath.h"

int32 main() {
    printf(c"=== Local Include Test ===\n");

    // Test 1: call function from local header
    int32 sum = add(10, 20);
    if (sum == 30) {
        printf(c"[PASS] add(10, 20) = %d\n", sum);
    } else {
        printf(c"[FAIL] add(10, 20) = %d, expected 30\n", sum);
    }

    // Test 2: another function from local header
    int32 prod = multiply(6, 7);
    if (prod == 42) {
        printf(c"[PASS] multiply(6, 7) = %d\n", prod);
    } else {
        printf(c"[FAIL] multiply(6, 7) = %d, expected 42\n", prod);
    }

    // Test 3: macro constant from local header
    int32 pi = PI_APPROX;
    if (pi == 314159) {
        printf(c"[PASS] PI_APPROX = %d\n", pi);
    } else {
        printf(c"[FAIL] PI_APPROX = %d, expected 314159\n", pi);
    }

    printf(c"=== Done ===\n");
    ret 0;
}
