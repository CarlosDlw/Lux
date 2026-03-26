namespace CharTest;

use std::log::println;
use std::ascii::isAlpha;
use std::ascii::isDigit;
use std::ascii::isAlphaNum;
use std::ascii::isUpper;
use std::ascii::isLower;
use std::ascii::isWhitespace;
use std::ascii::isPrintable;
use std::ascii::isControl;
use std::ascii::isHexDigit;
use std::ascii::isAscii;
use std::ascii::toUpper;
use std::ascii::toLower;
use std::ascii::toDigit;
use std::ascii::fromDigit;

int32 main() {
    // ── isAlpha ──────────────────────────────────────────────────────
    bool a1 = isAlpha('A');
    bool a2 = isAlpha('z');
    bool a3 = isAlpha('5');
    println(a1);         // expect: true
    println(a2);         // expect: true
    println(a3);         // expect: false

    // ── isDigit ──────────────────────────────────────────────────────
    bool d1 = isDigit('7');
    bool d2 = isDigit('a');
    println(d1);         // expect: true
    println(d2);         // expect: false

    // ── isAlphaNum ───────────────────────────────────────────────────
    bool an1 = isAlphaNum('X');
    bool an2 = isAlphaNum('3');
    bool an3 = isAlphaNum('!');
    println(an1);        // expect: true
    println(an2);        // expect: true
    println(an3);        // expect: false

    // ── isUpper / isLower ────────────────────────────────────────────
    bool u1 = isUpper('M');
    bool u2 = isUpper('m');
    bool l1 = isLower('m');
    bool l2 = isLower('M');
    println(u1);         // expect: true
    println(u2);         // expect: false
    println(l1);         // expect: true
    println(l2);         // expect: false

    // ── isWhitespace ─────────────────────────────────────────────────
    bool w1 = isWhitespace(' ');
    bool w2 = isWhitespace('x');
    println(w1);         // expect: true
    println(w2);         // expect: false

    // ── isPrintable ──────────────────────────────────────────────────
    bool p1 = isPrintable('~');
    bool p2 = isPrintable(' ');
    println(p1);         // expect: true
    println(p2);         // expect: true

    // ── isControl ────────────────────────────────────────────────────
    // '\n' is control char (0x0A)
    char ctrl = '\n';
    bool c1 = isControl(ctrl);
    bool c2 = isControl('A');
    println(c1);         // expect: true
    println(c2);         // expect: false

    // ── isHexDigit ───────────────────────────────────────────────────
    bool h1 = isHexDigit('f');
    bool h2 = isHexDigit('F');
    bool h3 = isHexDigit('9');
    bool h4 = isHexDigit('g');
    println(h1);         // expect: true
    println(h2);         // expect: true
    println(h3);         // expect: true
    println(h4);         // expect: false

    // ── isAscii ──────────────────────────────────────────────────────
    bool as1 = isAscii('Z');
    bool as2 = isAscii('0');
    println(as1);        // expect: true
    println(as2);        // expect: true

    // ── toUpper / toLower ────────────────────────────────────────────
    char up = toUpper('a');
    char lo = toLower('Z');
    char same = toUpper('5');
    println(up);         // expect: A
    println(lo);         // expect: z
    println(same);       // expect: 5

    // ── toDigit ──────────────────────────────────────────────────────
    int32 dig1 = toDigit('0');
    int32 dig2 = toDigit('9');
    int32 dig3 = toDigit('x');
    println(dig1);       // expect: 0
    println(dig2);       // expect: 9
    println(dig3);       // expect: -1

    // ── fromDigit ────────────────────────────────────────────────────
    char fd1 = fromDigit(0);
    char fd2 = fromDigit(5);
    char fd3 = fromDigit(9);
    println(fd1);        // expect: 0
    println(fd2);        // expect: 5
    println(fd3);        // expect: 9

    ret 0;
}
