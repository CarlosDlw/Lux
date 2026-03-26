namespace LogicalTest;

use std::log::println;

int32 main() {
    bool t = true;
    bool f = false;

    // Logical NOT
    bool not1 = !t;
    println(not1);

    bool not2 = !f;
    println(not2);

    // Logical AND
    bool and1 = t && t;
    println(and1);

    bool and2 = t && f;
    println(and2);

    bool and3 = f && f;
    println(and3);

    // Logical OR
    bool or1 = t || f;
    println(or1);

    bool or2 = f || f;
    println(or2);

    bool or3 = t || t;
    println(or3);

    // Combined with comparison
    int32 x = 10;
    int32 y = 20;
    bool comb = (x < y) && (y > 15);
    println(comb);

    bool comb2 = (x > 100) || (y == 20);
    println(comb2);

    // Chained logical
    bool chain = (x > 0) && (x < 100) && (y > 0);
    println(chain);

    ret 0;
}
