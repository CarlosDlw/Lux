namespace TryCatchFinallyTest;

use std::log::println;

int32 main() {
    try {
        println("before throw");
        throw Error { message: "something went wrong" };
        println("should not print");
    } catch (Error e) {
        println(e.message);
        println(e.file);
        println(e.line);
        println(e.column);
    } finally {
        println("finally block");
    }

    println("after try/catch");

    // Test unhandled throw
    // throw Error { message: "unhandled!" };

    ret 0;
}