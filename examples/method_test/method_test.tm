namespace MethodTest;

use std::log::println;

int32 main() {
    // Integer method
    int32 x = -42;
    int32 a = x.abs();

    // Float method
    float64 f = 3.14;
    float64 s = f.sqrt();

    // Bool method
    bool b = true;
    bool t = b.toggle();

    // String method
    string name = "hello";
    bool empty = name.isEmpty();

    // Char method
    char c = 'A';
    bool upper = c.isUpper();

    // Integer method with args
    int32 clamped = x.clamp(0, 100);

    ret 0;
}
