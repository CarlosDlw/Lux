namespace StructAbiTest;

#include <stdio.h>
#include "structs.h"

int32 main() {
    printf(c"=== Struct ABI Test ===\n");

    // Test 1: small struct return (8 bytes — 1 register)
    Point p = make_point(10, 20);
    int32 ps = point_sum(p);
    if (ps == 30) {
        printf(c"[PASS] point_sum(make_point(10,20)) = %d\n", ps);
    } else {
        printf(c"[FAIL] point_sum(make_point(10,20)) = %d, expected 30\n", ps);
    }

    // Test 2: medium struct return (16 bytes — 2 registers)
    Pair64 pr = make_pair64(100, 200);
    int64 prs = pair64_sum(pr);
    if (prs == 300) {
        printf(c"[PASS] pair64_sum(make_pair64(100,200)) = %ld\n", prs);
    } else {
        printf(c"[FAIL] pair64_sum(make_pair64(100,200)) = %ld, expected 300\n", prs);
    }

    // Test 3: large struct return (32 bytes — sret)
    Vec4 v = make_vec4(1, 2, 3, 4);
    int64 vs = vec4_sum(v);
    if (vs == 10) {
        printf(c"[PASS] vec4_sum(make_vec4(1,2,3,4)) = %ld\n", vs);
    } else {
        printf(c"[FAIL] vec4_sum(make_vec4(1,2,3,4)) = %ld, expected 10\n", vs);
    }

    // Test 4: nested struct (16 bytes — 2 registers)
    Rect r = make_rect(5, 10, 20, 30);
    int32 area = rect_area(r);
    if (area == 600) {
        printf(c"[PASS] rect_area(make_rect(5,10,20,30)) = %d\n", area);
    } else {
        printf(c"[FAIL] rect_area(make_rect(5,10,20,30)) = %d, expected 600\n", area);
    }

    printf(c"=== Done ===\n");
    ret 0;
}
