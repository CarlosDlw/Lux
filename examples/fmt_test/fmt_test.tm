namespace FmtTest;

use std::fmt::lpad;
use std::fmt::rpad;
use std::fmt::center;
use std::fmt::hex;
use std::fmt::hexUpper;
use std::fmt::oct;
use std::fmt::bin;
use std::fmt::fixed;
use std::fmt::scientific;
use std::fmt::humanBytes;
use std::fmt::commas;
use std::fmt::percent;
use std::log::println;

int32 main() {
    /* lpad */
    string lp1 = lpad("hi", 6, ' ');
    println(lp1);

    string lp2 = lpad("42", 5, '0');
    println(lp2);

    string lp3 = lpad("hello", 3, ' ');
    println(lp3);

    /* rpad */
    string rp1 = rpad("hi", 6, '.');
    println(rp1);

    string rp2 = rpad("hello", 3, ' ');
    println(rp2);

    /* center */
    string c1 = center("hi", 8, '-');
    println(c1);

    string c2 = center("X", 5, '*');
    println(c2);

    /* hex */
    string h1 = hex(255);
    println(h1);

    string h2 = hex(0);
    println(h2);

    string h3 = hex(4096);
    println(h3);

    /* hexUpper */
    string hu1 = hexUpper(255);
    println(hu1);

    string hu2 = hexUpper(48879);
    println(hu2);

    /* oct */
    string o1 = oct(8);
    println(o1);

    string o2 = oct(255);
    println(o2);

    /* bin */
    string b1 = bin(10);
    println(b1);

    string b2 = bin(255);
    println(b2);

    string b3 = bin(0);
    println(b3);

    /* fixed */
    string f1 = fixed(3.14159, 2);
    println(f1);

    string f2 = fixed(1.0, 4);
    println(f2);

    string f3 = fixed(100.0, 0);
    println(f3);

    /* scientific */
    string s1 = scientific(12345.6789);
    println(s1);

    string s2 = scientific(0.001);
    println(s2);

    /* humanBytes */
    string hb1 = humanBytes(500);
    println(hb1);

    string hb2 = humanBytes(1024);
    println(hb2);

    string hb3 = humanBytes(1536);
    println(hb3);

    string hb4 = humanBytes(1073741824);
    println(hb4);

    string hb5 = humanBytes(1099511627776);
    println(hb5);

    /* commas */
    string cm1 = commas(1000);
    println(cm1);

    string cm2 = commas(1000000);
    println(cm2);

    string cm3 = commas(1234567890);
    println(cm3);

    string cm4 = commas(-42000);
    println(cm4);

    string cm5 = commas(0);
    println(cm5);

    /* percent */
    string p1 = percent(0.855);
    println(p1);

    string p2 = percent(1.0);
    println(p2);

    string p3 = percent(0.5);
    println(p3);

    string p4 = percent(0.333);
    println(p4);

    println("all fmt tests passed");
    ret 0;
}
