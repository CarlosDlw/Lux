namespace ArgsTest;

use std::log::println;
use std::collections::Vec;

int32 main(Vec<string> args) {
    for string arg in args {
        println(arg);
    }

    ret 0;
}
