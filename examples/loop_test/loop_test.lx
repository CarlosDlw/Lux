namespace LoopTest;

use std::log::println;

int32 main() {
    int32 count = 0;

    loop {
        if count == 5 {
            break;
        }
        println(count);
        count += 1;
    }

    // loop with continue
    int32 i = 0;
    loop {
        if i == 8 {
            break;
        }
        i += 1;
        if i == 3 {
            continue;
        }
        if i == 6 {
            continue;
        }
        println(i);
    }

    ret 0;
}
