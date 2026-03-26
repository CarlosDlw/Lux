namespace Main007;

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

int32 main() {
    printf(c"=== #include Test Suite ===\n");

    // Test 1: printf from stdio.h (variadic)
    printf(c"[PASS] printf from #include <stdio.h>\n");

    // Test 2: puts from stdio.h
    puts(c"[PASS] puts from #include <stdio.h>");

    // Test 3: sqrt from math.h
    float64 s = sqrt(144.0);
    int32 si = s as int32;
    printf(c"[PASS] sqrt(144) = %d\n", si);

    // Test 4: sin from math.h
    float64 sn = sin(1.5707963267948966);
    printf(c"[PASS] sin(pi/2) = %f\n", sn);

    // Test 5: malloc + free from stdlib.h
    *void buf = malloc(64 as usize);
    memset(buf, 65, 5 as usize);  // fill with 'A'
    *char cbuf = buf as *char;
    printf(c"[PASS] malloc+memset: %c%c%c\n", (*cbuf) as int32, (*cbuf) as int32, (*cbuf) as int32);
    free(buf);

    // Test 6: strlen from string.h
    usize len = strlen(c"hello");
    printf(c"[PASS] strlen(\"hello\") = %d\n", len as int32);

    printf(c"=== All #include tests passed ===\n");
    ret 0;
}
