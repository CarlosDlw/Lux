namespace ThreadTest;

use std::log::println;
use std::thread::cpuCount;
use std::thread::threadId;
use std::thread::yield;
use std::thread::Task;
use std::thread::Mutex;
use std::test::assertTrue;
use std::test::assertEqual;
use std::test::log;

int32 compute(int32 a, int32 b) {
    ret a + b;
}

int32 heavyWork() {
    int32 sum = 0;
    for int32 i = 0; i < 1000; i++ {
        sum += i;
    }
    ret sum;
}

int32 main() {
    /* ── cpuCount ───────────────────────────────────────────────── */
    uint32 cores = cpuCount();
    assertTrue(cores > 0);
    log("cpuCount: PASS");

    /* ── threadId ───────────────────────────────────────────────── */
    uint64 tid = threadId();
    assertTrue(tid > 0);
    log("threadId: PASS");

    /* ── yield ──────────────────────────────────────────────────── */
    yield();
    log("yield: PASS");

    /* ── spawn + await (with args) ──────────────────────────────── */
    Task<int32> t1 = spawn compute(10, 20);
    int32 r1 = await t1;
    assertEqual(r1, 30);
    log("spawn+await with args: PASS");

    /* ── spawn + await (heavy work) ─────────────────────────────── */
    Task<int32> t2 = spawn heavyWork();
    Task<int32> t3 = spawn heavyWork();
    int32 r2 = await t2;
    int32 r3 = await t3;
    assertEqual(r2, 499500);
    assertEqual(r3, 499500);
    log("parallel spawn: PASS");

    /* ── Mutex + lock ───────────────────────────────────────────── */
    Mutex mtx = Mutex();
    int32 counter = 0;
    lock(mtx) {
        counter += 1;
    }
    assertEqual(counter, 1);
    log("lock/unlock: PASS");

    log("all thread tests passed");
    ret 0;
}
