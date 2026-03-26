namespace IntStd;

/*
    podemos importar módulos tollvm usando sintaxe similar a Rust.

    importação de std::log::println, std::log::print, std::log::{ println, print }, etc. também são suportados.

    os módulos builtins de tollvm são escritos em C, e estão localizados em src/builtins, separados por categorias:

    src/builtins/log: funções de log, como println, print, eprint, eprintln, etc.
*/
use std::log::println;

int32 main() {
    int1 a = 1; // 1-bit integer: 0 or 1
    int8 b = 10; // 8-bit integer: -128 to 127
    int16 c = 100; // 16-bit integer: -32768 to 32767
    int32 d = 1000; // 32-bit integer: -2147483648 to 2147483647
    int64 e = 10000; // 64-bit integer: -9223372036854775808 to 9223372036854775807
    int128 f = 100000; // 128-bit integer: -170141183460469231731687303715884105728 to 170141183460469231731687303715884105727
    intinf g = 1000000; // arbitrary precision integer
    isize h = 10000000; // pointer-sized integer: 32 or 64 bits depending on the target architecture

    println(a);
    println(b);
    println(c);
    println(d);
    println(e);
    println(f);
    println(g);
    println(h);
    
    ret 0;
}