namespace FsTest;

use std::fs::readFile;
use std::fs::writeFile;
use std::fs::appendFile;
use std::fs::exists;
use std::fs::isFile;
use std::fs::isDir;
use std::fs::remove;
use std::fs::removeDir;
use std::fs::rename;
use std::fs::mkdir;
use std::fs::mkdirAll;
use std::fs::fileSize;
use std::fs::cwd;
use std::fs::setCwd;
use std::fs::tempDir;
use std::log::println;

int32 main() {
    string testDir = "/tmp/tollvm_fs_test";
    string testFile = "/tmp/tollvm_fs_test/hello.txt";
    string testFile2 = "/tmp/tollvm_fs_test/renamed.txt";
    string subDir = "/tmp/tollvm_fs_test/a/b/c";

    /* cleanup from previous runs */
    remove(testFile);
    remove(testFile2);
    removeDir("/tmp/tollvm_fs_test/a/b/c");
    removeDir("/tmp/tollvm_fs_test/a/b");
    removeDir("/tmp/tollvm_fs_test/a");
    removeDir(testDir);

    /* mkdir */
    bool mkOk = mkdir(testDir);
    println(mkOk);

    /* exists / isDir / isFile on directory */
    println(exists(testDir));
    println(isDir(testDir));
    println(isFile(testDir));

    /* writeFile + readFile */
    writeFile(testFile, "hello world");
    string content = readFile(testFile);
    println(content);

    /* exists / isFile on file */
    println(exists(testFile));
    println(isFile(testFile));

    /* fileSize */
    int64 sz = fileSize(testFile);
    println(sz);

    /* appendFile */
    appendFile(testFile, "!!!");
    string content2 = readFile(testFile);
    println(content2);

    /* rename */
    bool renOk = rename(testFile, testFile2);
    println(renOk);
    println(exists(testFile));
    println(exists(testFile2));

    /* remove file */
    bool rmOk = remove(testFile2);
    println(rmOk);
    println(exists(testFile2));

    /* mkdirAll */
    bool mkAllOk = mkdirAll(subDir);
    println(mkAllOk);
    println(isDir(subDir));

    /* cwd */
    string curDir = cwd();
    bool hasCwd = fileSize("/proc/self/exe") > 0;
    println(hasCwd);

    /* setCwd + cwd */
    bool cdOk = setCwd("/tmp");
    println(cdOk);
    string newDir = cwd();
    println(newDir);

    /* restore cwd */
    setCwd(curDir);

    /* tempDir */
    string tmp = tempDir();
    println(isDir(tmp));

    /* cleanup */
    removeDir(subDir);
    removeDir("/tmp/tollvm_fs_test/a/b");
    removeDir("/tmp/tollvm_fs_test/a");
    removeDir(testDir);

    println("all fs tests passed");
    ret 0;
}
