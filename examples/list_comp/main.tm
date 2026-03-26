namespace list_comp;

extern void printf(cstring fmt, ...);

int32 main() {
    // Basic list comprehension
    [4]int32 arr = [x | for int32 x in 0..4];
    printf(c"basic:     %d %d %d %d\n", arr[0], arr[1], arr[2], arr[3]);

    // With expression transform
    [4]int32 squares = [x * x | for int32 x in 0..4];
    printf(c"squares:   %d %d %d %d\n", squares[0], squares[1], squares[2], squares[3]);

    // Inclusive range
    [5]int32 incl = [x | for int32 x in 1..=5];
    printf(c"inclusive:  %d %d %d %d %d\n", incl[0], incl[1], incl[2], incl[3], incl[4]);

    // With filter
    [8]int32 evens = [x | for int32 x in 0..8 if x % 2 == 0];
    printf(c"evens:     %d %d %d %d\n", evens[0], evens[1], evens[2], evens[3]);

    // Expression + filter
    [6]int32 oddSq = [x * x | for int32 x in 0..6 if x % 2 != 0];
    printf(c"odd sq:    %d %d %d\n", oddSq[0], oddSq[1], oddSq[2]);

    ret 0;
}
