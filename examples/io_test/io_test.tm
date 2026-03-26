namespace IoTest;

use std::log::println;
use std::io::prompt;
use std::io::promptInt;
use std::io::promptFloat;
use std::io::promptBool;
use std::io::isTTY;
use std::io::isStdoutTTY;
use std::io::flush;

int32 main() {
    // Terminal detection
    if isTTY() {
        println("stdin is a terminal");
    } else {
        println("stdin is piped");
    }

    if isStdoutTTY() {
        println("stdout is a terminal");
    } else {
        println("stdout is piped");
    }

    // Prompt-based input
    string name = prompt("Enter your name: ");
    println(name);

    int64 age = promptInt("Enter your age: ");
    println(age);

    float64 height = promptFloat("Enter your height: ");
    println(height);

    bool student = promptBool("Are you a student? (true/false): ");
    println(student);

    // Flush
    flush();

    ret 0;
}
