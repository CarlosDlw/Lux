namespace EnumTypeTest;

#include <stdio.h>
#include "colors.h"

int32 main() {
    printf(c"=== Enum Type Test ===\n");

    // Test 1: enum constant values
    int32 r = COLOR_RED;
    int32 g = COLOR_GREEN;
    int32 b = COLOR_BLUE;
    if (r == 0 && g == 1 && b == 2) {
        printf(c"[PASS] enum constants: R=%d G=%d B=%d\n", r, g, b);
    } else {
        printf(c"[FAIL] enum constants: R=%d G=%d B=%d\n", r, g, b);
    }

    // Test 2: pass enum to C function (CXType_Enum as parameter)
    *char name = color_name(COLOR_GREEN);
    printf(c"color_name(GREEN) = %s\n", name);

    // Test 3: return enum from C function (CXType_Enum as return)
    int32 next = next_color(COLOR_BLUE);
    if (next == 0) {
        printf(c"[PASS] next_color(BLUE) = %d (RED)\n", next);
    } else {
        printf(c"[FAIL] next_color(BLUE) = %d, expected 0\n", next);
    }

    printf(c"=== Done ===\n");
    ret 0;
}
