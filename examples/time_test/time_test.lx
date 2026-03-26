namespace TimeTest;

use std::log::println;
use std::time::now;
use std::time::nowNanos;
use std::time::nowMicros;
use std::time::sleep;
use std::time::sleepMicros;
use std::time::clock;
use std::time::year;
use std::time::month;
use std::time::day;
use std::time::hour;
use std::time::minute;
use std::time::second;
use std::time::weekday;
use std::time::timestamp;
use std::time::elapsed;
use std::time::formatTime;
use std::time::parseTime;

int32 main() {
    // Test 1: now — should return a reasonable timestamp (> 2020-01-01 in ms)
    uint64 t = now();
    println(t > 1577836800000);     // expected: true

    // Test 2: nowNanos — should be > now * 1000000
    uint64 tn = nowNanos();
    println(tn > t * 1000);        // expected: true

    // Test 3: nowMicros — should be between now*1000 and nanos
    uint64 tu = nowMicros();
    println(tu > t * 1000);        // expected: true (approx)

    // Test 4: clock — monotonic, should be > 0
    uint64 c = clock();
    println(c > 0);                // expected: true

    // Test 5: year — should be >= 2026
    int32 y = year();
    println(y >= 2026);            // expected: true

    // Test 6: month — should be 1-12
    int32 mo = month();
    println(mo >= 1);              // expected: true
    println(mo <= 12);             // expected: true

    // Test 7: day — should be 1-31
    int32 d = day();
    println(d >= 1);               // expected: true
    println(d <= 31);              // expected: true

    // Test 8: hour — should be 0-23
    int32 h = hour();
    println(h >= 0);               // expected: true
    println(h <= 23);              // expected: true

    // Test 9: minute — should be 0-59
    int32 mi = minute();
    println(mi >= 0);              // expected: true
    println(mi <= 59);             // expected: true

    // Test 10: second — should be 0-59
    int32 s = second();
    println(s >= 0);               // expected: true
    println(s <= 59);              // expected: true

    // Test 11: weekday — should be 0-6
    int32 wd = weekday();
    println(wd >= 0);              // expected: true
    println(wd <= 6);              // expected: true

    // Test 12: timestamp — should be a non-empty string
    string ts = timestamp();
    println(ts);                   // expected: ISO 8601 format (e.g. 2026-03-23T14:30:45)

    // Test 13: sleep — should not crash (sleep 10ms)
    uint64 before = now();
    sleep(10);
    uint64 after = now();
    println(after >= before);      // expected: true

    // Test 14: sleepMicros — should not crash (sleep 1000us = 1ms)
    sleepMicros(1000);
    println(true);                 // expected: true (no crash)

    // Test 15: elapsed — should measure time passed
    uint64 start = now();
    sleep(15);
    uint64 e = elapsed(start);
    println(e >= 10);              // expected: true (at least 10ms)

    // Test 16: formatTime — format a timestamp
    // Use a known timestamp: 2000-01-01 00:00:00 UTC = 946684800000 ms
    string formatted = formatTime(946684800000, "%Y-%m-%d");
    println(formatted);            // expected: date near 2000-01-01 (varies by TZ)

    // Test 17: parseTime — parse a date string
    uint64 parsed = parseTime("2000-06-15", "%Y-%m-%d");
    println(parsed > 0);           // expected: true

    println("all time tests passed");

    ret 0;
}
