namespace PointerStructTest;

use std::log::println;

// Self-referencing struct (linked list node)
struct Node {
    int32 value;
    *Node next;
}

// Cross-struct pointer
struct Inner {
    int32 x;
    int32 y;
}

struct Outer {
    int32 id;
    *Inner data;
}

int32 main() {
    // Test 1: self-referencing struct
    Node c = Node { value: 30, next: null };
    Node b = Node { value: 20, next: &c };
    Node a = Node { value: 10, next: &b };

    println(a.value);
    println(a.next->value);
    println(a.next->next->value);

    // Test 2: cross-struct pointer
    Inner point = Inner { x: 100, y: 200 };
    Outer wrapper = Outer { id: 1, data: &point };

    println(wrapper.id);
    println(wrapper.data->x);
    println(wrapper.data->y);

    ret 0;
}
