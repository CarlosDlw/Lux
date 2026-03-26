namespace ArrowTest;

use std::log::println;

struct Point {
    int32 x;
    int32 y;
}

int32 main() {
    Point p = Point { x: 10, y: 20 };
    *Point ptr = &p;

    // arrow access
    println(ptr->x);
    println(ptr->y);

    // dot access for comparison
    println(p.x);
    println(p.y);

    ret 0;
}
