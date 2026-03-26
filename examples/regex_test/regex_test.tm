namespace RegexTest;

use std::regex::match;
use std::regex::find;
use std::regex::findIndex;
use std::regex::regexReplace;
use std::regex::regexReplaceFirst;
use std::regex::isValid;
use std::log::println;

int32 main() {
    /* match — partial match (contains) */
    bool m1 = match("hello world", "hello");
    println(m1);

    bool m2 = match("hello world", "^hello");
    println(m2);

    bool m3 = match("hello world", "^world");
    println(m3);

    bool m4 = match("abc123", "[0-9]+");
    println(m4);

    bool m5 = match("abc", "[0-9]+");
    println(m5);

    bool m6 = match("foo@bar.com", "[a-z]+@[a-z]+.[a-z]+");
    println(m6);

    /* find — first matched substring */
    string f1 = find("hello 42 world 99", "[0-9]+");
    println(f1);

    string f2 = find("no digits here", "[0-9]+");
    println(f2);

    string f3 = find("abc-def-ghi", "[a-z]+");
    println(f3);

    /* findIndex */
    int64 fi1 = findIndex("hello world", "world");
    println(fi1);

    int64 fi2 = findIndex("hello world", "xyz");
    println(fi2);

    int64 fi3 = findIndex("abc123def", "[0-9]+");
    println(fi3);

    /* regexReplace — replace all */
    string r1 = regexReplace("hello 42 world 99", "[0-9]+", "N");
    println(r1);

    string r2 = regexReplace("a-b-c", "-", "_");
    println(r2);

    string r3 = regexReplace("  hello   world  ", " +", " ");
    println(r3);

    /* regexReplaceFirst — replace first only */
    string rf1 = regexReplaceFirst("aaa bbb ccc", "[a-z]+", "X");
    println(rf1);

    string rf2 = regexReplaceFirst("123-456-789", "[0-9]+", "N");
    println(rf2);

    /* isValid */
    bool v1 = isValid("[a-z]+");
    println(v1);

    bool v2 = isValid("[invalid");
    println(v2);

    bool v3 = isValid("(hello|world)");
    println(v3);

    bool v4 = isValid("(unclosed");
    println(v4);

    println("all regex tests passed");
    ret 0;
}
