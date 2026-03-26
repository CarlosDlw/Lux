namespace CompressTest;

use std::log::println;
use std::log::print;
use std::compress::gzipCompress;
use std::compress::gzipDecompress;
use std::compress::deflate;
use std::compress::inflate;
use std::compress::compressLevel;
use std::test::assertTrue;
use std::test::assertEqual;
use std::test::assertLess;
use std::test::log;

int32 main() {
    string original = "Hello, World! This is a test of the compression module. We need enough data to see meaningful compression ratios. AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    /* ── gzip round-trip ─────────────────────────────────────────── */
    string compressed = gzipCompress(original);
    string decompressed = gzipDecompress(compressed);
    assertEqual(original, decompressed);
    log("gzip round-trip: PASS");

    /* ── raw deflate round-trip ──────────────────────────────────── */
    string rawComp = deflate(original);
    string rawDecomp = inflate(rawComp);
    assertEqual(original, rawDecomp);
    log("deflate round-trip: PASS");

    /* ── compressLevel ───────────────────────────────────────────── */
    string lvl1 = compressLevel(original, 1);
    string lvl9 = compressLevel(original, 9);

    string dec1 = gzipDecompress(lvl1);
    string dec9 = gzipDecompress(lvl9);
    assertEqual(original, dec1);
    log("level 1 round-trip: PASS");
    assertEqual(original, dec9);
    log("level 9 round-trip: PASS");

    /* ── compression actually reduces size ───────────────────────── */
    assertLess(compressed.len(), original.len());
    log("compression reduces size: PASS");

    /* ── deflate should be smaller than gzip (no header) ─────────── */
    assertLess(rawComp.len(), compressed.len());
    log("deflate < gzip size: PASS");

    log("all compression tests passed");
    ret 0;
}
