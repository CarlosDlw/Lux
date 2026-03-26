namespace StringTest;

use std::log::println;

use std::str::contains;
use std::str::startsWith;
use std::str::endsWith;
use std::str::indexOf;
use std::str::lastIndexOf;
use std::str::count;
use std::str::toUpper;
use std::str::toLower;
use std::str::trim;
use std::str::trimLeft;
use std::str::trimRight;
use std::str::replace;
use std::str::replaceFirst;
use std::str::repeat;
use std::str::reverse;
use std::str::padLeft;
use std::str::padRight;
use std::str::substring;
use std::str::charAt;
use std::str::slice;
use std::str::parseInt;
use std::str::parseIntRadix;
use std::str::parseFloat;
use std::str::fromCharCode;

int32 main() {
    // ── Search & Match ──────────────────────────────────────────────
    string text = "Hello, World!";

    println(contains(text, "World"));
    println(contains(text, "world"));
    println(startsWith(text, "Hello"));
    println(endsWith(text, "World!"));
    println(indexOf(text, "World"));
    println(lastIndexOf(text, "l"));
    println(count(text, "l"));

    // ── Transformation ──────────────────────────────────────────────
    println(toUpper("hello"));
    println(toLower("HELLO"));
    println(trim("  hello  "));
    println(trimLeft("  hello  "));
    println(trimRight("  hello  "));

    // ── Replace ─────────────────────────────────────────────────────
    println(replace("aabbcc", "b", "X"));
    println(replaceFirst("aabbcc", "b", "X"));
    println(repeat("ab", 3));
    println(reverse("Hello"));

    // ── Padding ─────────────────────────────────────────────────────
    println(padLeft("42", 6, '0'));
    println(padRight("Hi", 6, '.'));

    // ── Extraction ──────────────────────────────────────────────────
    println(substring("Hello, World!", 7, 5));
    char ch = charAt("Hello", 1);
    println(ch);
    println(slice("Hello, World!", 7, 12));
    println(slice("Hello, World!", -6, -1));

    // ── Parsing ─────────────────────────────────────────────────────
    println(parseInt("42"));
    println(parseIntRadix("ff", 16));
    println(parseIntRadix("1010", 2));
    println(parseFloat("3.14"));

    // ── Conversion ──────────────────────────────────────────────────
    char a = fromCharCode(65);
    println(a);

    ret 0;
}
