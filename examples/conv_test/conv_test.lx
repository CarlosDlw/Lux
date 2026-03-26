namespace ConvTest;

use std::conv::itoa;
use std::conv::itoaRadix;
use std::conv::utoa;
use std::conv::ftoa;
use std::conv::ftoaPrecision;
use std::conv::atoi;
use std::conv::atof;
use std::conv::toHex;
use std::conv::toOctal;
use std::conv::toBinary;
use std::conv::fromHex;
use std::conv::charToInt;
use std::conv::intToChar;
use std::log::println;

int32 main() {
    /* itoa */
    string s1 = itoa(42);
    println(s1);

    string s2 = itoa(-123);
    println(s2);

    /* itoaRadix */
    string s3 = itoaRadix(255, 16);
    println(s3);

    string s4 = itoaRadix(10, 2);
    println(s4);

    string s5 = itoaRadix(8, 8);
    println(s5);

    string s6 = itoaRadix(-42, 10);
    println(s6);

    /* utoa */
    uint64 uv = 999;
    string s7 = utoa(uv);
    println(s7);

    /* ftoa */
    string s8 = ftoa(3.14);
    println(s8);

    string s9 = ftoa(100.0);
    println(s9);

    /* ftoaPrecision */
    uint32 prec = 2;
    string s10 = ftoaPrecision(3.14159, prec);
    println(s10);

    uint32 prec4 = 4;
    string s11 = ftoaPrecision(1.0, prec4);
    println(s11);

    /* atoi */
    int64 n1 = atoi("456");
    println(n1);

    int64 n2 = atoi("-789");
    println(n2);

    /* atof */
    float64 f1 = atof("2.718");
    println(f1);

    /* toHex */
    string h1 = toHex(255);
    println(h1);

    string h2 = toHex(0);
    println(h2);

    /* toOctal */
    string o1 = toOctal(8);
    println(o1);

    /* toBinary */
    string b1 = toBinary(10);
    println(b1);

    string b2 = toBinary(1);
    println(b2);

    string b3 = toBinary(0);
    println(b3);

    /* fromHex */
    uint64 fh1 = fromHex("ff");
    println(fh1);

    uint64 fh2 = fromHex("1a");
    println(fh2);

    /* charToInt */
    char c = 'A';
    int32 code = charToInt(c);
    println(code);

    /* intToChar */
    int32 codeB = 66;
    char ch = intToChar(codeB);
    println(ch);

    println("all conv tests passed");
    ret 0;
}
