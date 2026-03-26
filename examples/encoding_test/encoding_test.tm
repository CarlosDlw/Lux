namespace EncodingTest;

use std::encoding::base64EncodeStr;
use std::encoding::base64DecodeStr;
use std::encoding::urlEncode;
use std::encoding::urlDecode;
use std::log::println;

int32 main() {
    // ── Base64 encode ──────────────────────────────────────────
    string b1 = base64EncodeStr("");
    println(b1);                          // (empty)

    string b2 = base64EncodeStr("H");
    println(b2);                          // SA==

    string b3 = base64EncodeStr("He");
    println(b3);                          // SGU=

    string b4 = base64EncodeStr("Hel");
    println(b4);                          // SGVs

    string b5 = base64EncodeStr("Hello");
    println(b5);                          // SGVsbG8=

    string b6 = base64EncodeStr("Hello, World!");
    println(b6);                          // SGVsbG8sIFdvcmxkIQ==

    string b7 = base64EncodeStr("Man");
    println(b7);                          // TWFu

    string b8 = base64EncodeStr("Ma");
    println(b8);                          // TWE=

    string b9 = base64EncodeStr("M");
    println(b9);                          // TQ==

    // ── Base64 decode ──────────────────────────────────────────
    string d1 = base64DecodeStr("");
    println(d1);                          // (empty)

    string d2 = base64DecodeStr("SGVsbG8=");
    println(d2);                          // Hello

    string d3 = base64DecodeStr("SGVsbG8sIFdvcmxkIQ==");
    println(d3);                          // Hello, World!

    string d4 = base64DecodeStr("TWFu");
    println(d4);                          // Man

    string d5 = base64DecodeStr("TWE=");
    println(d5);                          // Ma

    string d6 = base64DecodeStr("TQ==");
    println(d6);                          // M

    // ── URL encode ─────────────────────────────────────────────
    string u1 = urlEncode("hello world");
    println(u1);                          // hello%20world

    string u2 = urlEncode("foo=bar&baz=qux");
    println(u2);                          // foo%3Dbar%26baz%3Dqux

    string u3 = urlEncode("simple");
    println(u3);                          // simple

    string u4 = urlEncode("100% done!");
    println(u4);                          // 100%25%20done%21

    string u5 = urlEncode("a+b=c");
    println(u5);                          // a%2Bb%3Dc

    string u6 = urlEncode("path/to/file");
    println(u6);                          // path%2Fto%2Ffile

    string u7 = urlEncode("");
    println(u7);                          // (empty)

    // ── URL decode ─────────────────────────────────────────────
    string v1 = urlDecode("hello%20world");
    println(v1);                          // hello world

    string v2 = urlDecode("foo%3Dbar%26baz%3Dqux");
    println(v2);                          // foo=bar&baz=qux

    string v3 = urlDecode("simple");
    println(v3);                          // simple

    string v4 = urlDecode("100%25%20done%21");
    println(v4);                          // 100% done!

    string v5 = urlDecode("a%2Bb%3Dc");
    println(v5);                          // a+b=c

    string v6 = urlDecode("hello+world");
    println(v6);                          // hello world

    ret 0;
}
