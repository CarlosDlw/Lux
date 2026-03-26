namespace OsTest;

use std::os::getpid;
use std::os::getppid;
use std::os::getuid;
use std::os::getgid;
use std::os::hostname;
use std::os::pageSize;
use std::os::errno;
use std::os::strerror;
use std::os::dup;
use std::os::dup2;
use std::os::closeFd;
use std::log::println;

int32 main() {
    // ── getpid / getppid ───────────────────────────────────────
    int32 myPid = getpid();
    bool pidOk = myPid > 0;
    println(pidOk);                       // true

    int32 parentPid = getppid();
    bool ppidOk = parentPid > 0;
    println(ppidOk);                      // true

    // ── getuid / getgid ────────────────────────────────────────
    uint32 uid = getuid();
    println(uid > 0);                     // true

    uint32 gid = getgid();
    println(gid > 0);                     // true

    // ── hostname ───────────────────────────────────────────────
    string host = hostname();
    println(host);                        // (hostname, dynamic)

    // ── pageSize ───────────────────────────────────────────────
    usize ps = pageSize();
    println(ps > 0);                      // true

    // ── errno / strerror ───────────────────────────────────────
    int32 err = errno();
    println(err);                         // 0

    string msg = strerror(2);
    println(msg);                         // No such file or directory

    string msg2 = strerror(13);
    println(msg2);                        // Permission denied

    string msg3 = strerror(0);
    println(msg3);                        // Success

    // ── dup / dup2 / closeFd ───────────────────────────────────
    int32 newFd = dup(1);
    println(newFd > 2);                   // true

    int32 closeRet = closeFd(newFd);
    println(closeRet);                    // 0

    int32 targetFd = 42;
    int32 d2Ret = dup2(1, targetFd);
    println(d2Ret);                       // 42

    int32 closeRet2 = closeFd(targetFd);
    println(closeRet2);                   // 0

    int32 badClose = closeFd(999);
    println(badClose);                    // -1

    ret 0;
}
