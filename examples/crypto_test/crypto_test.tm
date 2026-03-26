namespace CryptoTest;

use std::crypto::md5String;
use std::crypto::sha1String;
use std::crypto::sha256String;
use std::crypto::sha512String;
use std::log::println;

int32 main() {
    // ── MD5 ────────────────────────────────────────────────────
    println(md5String(""));
    println(md5String("hello"));
    println(md5String("The quick brown fox jumps over the lazy dog"));

    // ── SHA-1 ──────────────────────────────────────────────────
    println(sha1String(""));
    println(sha1String("hello"));
    println(sha1String("The quick brown fox jumps over the lazy dog"));

    // ── SHA-256 ────────────────────────────────────────────────
    println(sha256String(""));
    println(sha256String("hello"));
    println(sha256String("The quick brown fox jumps over the lazy dog"));

    // ── SHA-512 ────────────────────────────────────────────────
    println(sha512String(""));
    println(sha512String("hello"));
    println(sha512String("The quick brown fox jumps over the lazy dog"));

    ret 0;
}
