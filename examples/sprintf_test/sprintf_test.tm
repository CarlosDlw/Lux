namespace SprintfTest;

use std::log::{ println, sprintf };

int32 main() {
    string hello = sprintf("Hello, {}!", "World");
    println(hello);

    string math = sprintf("{} + {} = {}", toString(2), toString(3), toString(5));
    println(math);

    string single = sprintf("No placeholders here");
    println(single);

    string extra = sprintf("Only one: {}", "used");
    println(extra);

    string concat = sprintf("{}{}{}", "a", "b", "c");
    println(concat);

    ret 0;
}
