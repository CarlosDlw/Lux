namespace MacroTest;

#include <stdio.h>
#include <stdlib.h>

int32 main() {
    printf(c"=== #define Macro Constants Test ===\n");

    // Test 1: EOF (typically -1)
    int32 eof = EOF;
    printf(c"EOF = %d ", eof);
    if (eof == -1) {
        printf(c"[PASS]\n");
    } else {
        printf(c"[FAIL]\n");
    }

    // Test 2: EXIT_SUCCESS (typically 0)
    int32 es = EXIT_SUCCESS;
    printf(c"EXIT_SUCCESS = %d ", es);
    if (es == 0) {
        printf(c"[PASS]\n");
    } else {
        printf(c"[FAIL]\n");
    }

    // Test 3: EXIT_FAILURE (typically 1)
    int32 ef = EXIT_FAILURE;
    printf(c"EXIT_FAILURE = %d ", ef);
    if (ef == 1) {
        printf(c"[PASS]\n");
    } else {
        printf(c"[FAIL]\n");
    }

    // Test 4: BUFSIZ (usually 8192)
    int32 bs = BUFSIZ;
    printf(c"BUFSIZ = %d ", bs);
    if (bs > 0) {
        printf(c"[PASS]\n");
    } else {
        printf(c"[FAIL]\n");
    }

    // Test 5: SEEK_SET, SEEK_CUR, SEEK_END
    int32 ss = SEEK_SET;
    int32 sc = SEEK_CUR;
    int32 se = SEEK_END;
    printf(c"SEEK_SET=%d SEEK_CUR=%d SEEK_END=%d ", ss, sc, se);
    if (ss == 0) {
        printf(c"[PASS]\n");
    } else {
        printf(c"[FAIL]\n");
    }

    printf(c"=== All macro tests done ===\n");
    ret EXIT_SUCCESS;
}
