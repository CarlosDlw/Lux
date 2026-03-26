namespace BitsTest;

use std::bits::popcount;
use std::bits::ctz;
use std::bits::clz;
use std::bits::rotl;
use std::bits::rotr;
use std::bits::bswap;
use std::bits::isPow2;
use std::bits::nextPow2;
use std::bits::bitReverse;
use std::bits::extractBits;
use std::bits::setBit;
use std::bits::clearBit;
use std::bits::toggleBit;
use std::bits::testBit;
use std::log::println;

int32 main() {
    /* popcount */
    uint64 v1 = 255;
    uint32 pc = popcount(v1);
    println(pc);

    uint32 pc0 = popcount(0);
    println(pc0);

    /* ctz */
    uint64 v2 = 8;
    uint32 tz = ctz(v2);
    println(tz);

    uint32 tz0 = ctz(0);
    println(tz0);

    /* clz */
    uint64 v3 = 1;
    uint32 lz = clz(v3);
    println(lz);

    /* rotl / rotr */
    uint64 v4 = 1;
    uint32 shift = 4;
    uint64 rl = rotl(v4, shift);
    println(rl);

    uint64 rr = rotr(rl, shift);
    println(rr);

    /* bswap */
    uint64 v5 = 1;
    uint64 bs = bswap(v5);
    println(bs);

    uint64 bs2 = bswap(bs);
    println(bs2);

    /* isPow2 */
    println(isPow2(16));
    println(isPow2(15));
    println(isPow2(0));
    println(isPow2(1));

    /* nextPow2 */
    uint64 np1 = nextPow2(5);
    println(np1);

    uint64 np2 = nextPow2(16);
    println(np2);

    uint64 np3 = nextPow2(0);
    println(np3);

    /* bitReverse */
    uint64 br1 = bitReverse(1);
    uint64 expected = 9223372036854775808;
    println(br1 == expected);

    /* extractBits: extract bits 4..7 from 0xFF (= 0xF0 >> 4 = 0xF = 15) */
    uint64 v6 = 255;
    uint32 pos = 4;
    uint32 width = 4;
    uint64 eb = extractBits(v6, pos, width);
    println(eb);

    /* setBit / clearBit / toggleBit / testBit */
    uint64 v7 = 0;
    uint32 bit3 = 3;
    uint64 s = setBit(v7, bit3);
    println(s);

    println(testBit(s, bit3));

    uint64 c = clearBit(s, bit3);
    println(c);

    println(testBit(c, bit3));

    uint64 t = toggleBit(c, bit3);
    println(t);

    uint64 t2 = toggleBit(t, bit3);
    println(t2);

    println("all bits tests passed");
    ret 0;
}
