namespace NetTest;

use std::net::resolve;
use std::net::udpBind;
use std::net::udpSendTo;
use std::net::udpRecvFrom;
use std::net::tcpListen;
use std::net::close;
use std::net::setTimeout;
use std::test::assertTrue;
use std::test::assertNotEqual;
use std::test::assertEqual;
use std::test::log;

int32 main() {
    /* ── resolve ────────────────────────────────────────────────── */
    string ip = resolve("localhost");
    assertNotEqual(ip, "");
    log("resolve localhost: PASS");

    /* ── UDP loopback self-send ──────────────────────────────────── */
    int32 udpFd = udpBind("127.0.0.1", 39876);
    assertTrue(udpFd > 0);
    log("udpBind: PASS");

    setTimeout(udpFd, 2000);
    log("setTimeout: PASS");

    int64 sent = udpSendTo(udpFd, "hello udp", "127.0.0.1", 39876);
    assertTrue(sent > 0);
    log("udpSendTo: PASS");

    string received = udpRecvFrom(udpFd, 1024);
    assertEqual(received, "hello udp");
    log("udpRecvFrom: PASS");

    close(udpFd);
    log("close udp: PASS");

    /* ── TCP listen / close ─────────────────────────────────────── */
    int32 tcpFd = tcpListen("127.0.0.1", 39877);
    assertTrue(tcpFd > 0);
    log("tcpListen: PASS");

    close(tcpFd);
    log("close tcp: PASS");

    log("all net tests passed");
    ret 0;
}
