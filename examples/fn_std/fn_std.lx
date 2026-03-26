namespace FnStd;

use std::log::println;

type BinOp = fn(int32, int32) -> int32;

int32 add(int32 a, int32 b) {
    ret a + b;
}

int32 mul(int32 a, int32 b) {
    ret a * b;
}

int32 apply(BinOp op, int32 x, int32 y) {
    ret op(x, y);
}

int32 main() {
    BinOp op = add;
    println(op(3, 4));

    println(apply(mul, 5, 6));

    op = mul;
    println(op(3, 4));

    ret 0;
}
