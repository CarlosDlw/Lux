namespace ForTest;

use std::log::println;

int32 main() {
    // 1) C-style for
    for int32 i = 0; i < 5; i++ {
        println(i);
    }

    // 2) for..in range exclusive (0..5 = 0,1,2,3,4)
    for int32 i in 0..5 {
        println(i);
    }

    // 3) for..in range inclusive (0..=3 = 0,1,2,3)
    for int32 i in 0..=3 {
        println(i);
    }

    // 4) for..in array
    []int32 arr = [10, 20, 30, 40, 50];
    for int32 x in arr {
        println(x);
    }

    // 5) break
    for int32 i in 0..100 {
        if i == 3 {
            break;
        }
        println(i);
    }

    // 6) continue
    for int32 i in 0..6 {
        if i == 2 {
            continue;
        }
        if i == 4 {
            continue;
        }
        println(i);
    }

    ret 0;
}
