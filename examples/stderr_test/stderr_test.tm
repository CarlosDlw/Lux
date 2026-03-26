namespace StderrTest;

use std::log::{ println, eprintln, eprint, dbg };

int32 main() {
    // eprintln: prints to stderr with newline
    eprintln(42);
    eprintln("error message");
    eprintln(true);
    eprintln(3.14);
    eprintln('X');

    // eprint: prints to stderr without newline
    eprint("code: ");
    eprintln(404);

    // dbg: prints debug info to stderr and returns value
    int32 x = dbg(100);
    println(x);

    dbg("hello");
    dbg(false);
    dbg(2.718);

    ret 0;
}
