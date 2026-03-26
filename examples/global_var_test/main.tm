namespace GlobalVarTest;

#include <stdio.h>
#include <stdlib.h>

int32 main() {
    printf(c"=== C Global Variables Test ===\n");

    // Test 1: stdout is accessible as a C global variable
    fprintf(stdout, c"[PASS] fprintf to stdout works\n");

    // Test 2: stderr is accessible as a C global variable
    fprintf(stderr, c"[PASS] fprintf to stderr works\n");

    // Test 3: We can pass stdout/stderr to functions expecting FILE*
    fputs(c"[PASS] fputs to stdout works\n", stdout);

    // Test 4: Using macro constants alongside globals
    int32 es = EXIT_SUCCESS;
    fprintf(stdout, c"EXIT_SUCCESS = %d [PASS]\n", es);

    printf(c"=== All global var tests done ===\n");
    ret 0;
}
