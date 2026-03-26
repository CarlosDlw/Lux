namespace TestVecBuiltins;

use std::log::println;
use std::collections::Vec;
use std::str::split;
use std::str::joinVec;
use std::str::lines;
use std::str::chars;
use std::str::fromChars;
use std::str::toBytes;
use std::str::fromBytes;
use std::hash::hashBytes;
use std::hash::crc32;
use std::encoding::base64Encode;
use std::encoding::base64Decode;
use std::crypto::sha256;
use std::crypto::randomBytes;
use std::path::joinAll;
use std::regex::findAll;
use std::regex::regexSplit;
use std::fs::listDir;
use std::collections::Vec;

int32 main() {
    // ── split / joinVec ─────────────────────────────────────────────
    Vec<string> parts = split("hello,world,foo", ",");
    int64 partsLen = parts.len();
    println(partsLen);

    string joined = joinVec(parts, " | ");
    println(joined);

    // ── lines ───────────────────────────────────────────────────────
    Vec<string> ls = lines("line1\nline2\nline3");
    int64 lsLen = ls.len();
    println(lsLen);

    // ── chars / fromChars ────────────────────────────────────────────
    Vec<char> cs = chars("abc");
    int64 csLen = cs.len();
    println(csLen);

    string back = fromChars(cs);
    println(back);

    // ── toBytes / fromBytes ──────────────────────────────────────────
    Vec<uint8> bs = toBytes("hello");
    int64 bsLen = bs.len();
    println(bsLen);

    string strBack = fromBytes(bs);
    println(strBack);

    // ── hashBytes / crc32 ────────────────────────────────────────────
    Vec<uint8> data = toBytes("hello world");
    uint64 h = hashBytes(data);
    println(h != 0);

    uint32 crcVal = crc32(data);
    println(crcVal != 0);

    // ── base64Encode / base64Decode ──────────────────────────────────
    Vec<uint8> rawBytes = toBytes("hello");
    string encoded = base64Encode(rawBytes);
    println(encoded);

    Vec<uint8> decoded = base64Decode(encoded);
    string decodedStr = fromBytes(decoded);
    println(decodedStr);

    // ── sha256 (bytes) ───────────────────────────────────────────────
    Vec<uint8> hashInput = toBytes("abc");
    string sha = sha256(hashInput);
    int64 shaLen = sha.len();
    println(shaLen);

    // ── randomBytes ──────────────────────────────────────────────────
    Vec<uint8> rnd = randomBytes(16);
    int64 rndLen = rnd.len();
    println(rndLen);

    // ── findAll ──────────────────────────────────────────────────────
    Vec<string> matches = findAll("foo bar baz", "[a-z]+");
    int64 matchCount = matches.len();
    println(matchCount);

    // ── regexSplit ───────────────────────────────────────────────────
    Vec<string> rsParts = regexSplit("one  two   three", " +");
    int64 rsCount = rsParts.len();
    println(rsCount);

    // ── joinAll (path) ───────────────────────────────────────────────
    Vec<string> pathParts = ["usr", "local", "bin"];
    string fullPath = joinAll(pathParts);
    println(fullPath);

    // ── listDir ──────────────────────────────────────────────────────
    Vec<string> entries = listDir(".");
    bool hasEntries = entries.len() > 0;
    println(hasEntries);

    ret 0;
}
