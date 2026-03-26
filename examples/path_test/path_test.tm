namespace PathTest;

use std::path::join;
use std::path::parent;
use std::path::fileName;
use std::path::stem;
use std::path::extension;
use std::path::isAbsolute;
use std::path::isRelative;
use std::path::normalize;
use std::path::toAbsolute;
use std::path::separator;
use std::path::withExtension;
use std::path::withFileName;
use std::log::println;

int32 main() {
    /* join */
    string j1 = join("/home/user", "documents");
    println(j1);

    string j2 = join("/home/user/", "documents");
    println(j2);

    string j3 = join("a", "/absolute/path");
    println(j3);

    string j4 = join("/home", "");
    println(j4);

    /* parent */
    string p1 = parent("/home/user/file.txt");
    println(p1);

    string p2 = parent("/home");
    println(p2);

    string p3 = parent("file.txt");
    println(p3);

    string p4 = parent("/");
    println(p4);

    /* fileName */
    string f1 = fileName("/home/user/file.txt");
    println(f1);

    string f2 = fileName("file.txt");
    println(f2);

    string f3 = fileName("/home/user/");
    println(f3);

    /* stem */
    string s1 = stem("/home/user/file.txt");
    println(s1);

    string s2 = stem("archive.tar.gz");
    println(s2);

    string s3 = stem("noext");
    println(s3);

    string s4 = stem(".hidden");
    println(s4);

    /* extension */
    string e1 = extension("/home/user/file.txt");
    println(e1);

    string e2 = extension("archive.tar.gz");
    println(e2);

    string e3 = extension("noext");
    println(e3);

    string e4 = extension(".hidden");
    println(e4);

    /* isAbsolute / isRelative */
    bool abs1 = isAbsolute("/home/user");
    println(abs1);

    bool abs2 = isAbsolute("relative/path");
    println(abs2);

    bool rel1 = isRelative("relative/path");
    println(rel1);

    bool rel2 = isRelative("/absolute");
    println(rel2);

    /* normalize */
    string n1 = normalize("/home/user/../docs/./file.txt");
    println(n1);

    string n2 = normalize("a/b/../c/./d");
    println(n2);

    string n3 = normalize("/a/b/../../c");
    println(n3);

    /* toAbsolute — result will be cwd-dependent, so just check it starts with / */
    string ta = toAbsolute("relative");
    bool taAbs = isAbsolute(ta);
    println(taAbs);

    /* separator */
    char sep = separator();
    println(sep);

    /* withExtension */
    string we1 = withExtension("/home/file.txt", "md");
    println(we1);

    string we2 = withExtension("/home/file.txt", ".rs");
    println(we2);

    string we3 = withExtension("archive.tar.gz", "zip");
    println(we3);

    /* withFileName */
    string wf1 = withFileName("/home/user/old.txt", "new.txt");
    println(wf1);

    string wf2 = withFileName("/home/user/old.txt", "readme.md");
    println(wf2);

    println("all path tests passed");
    ret 0;
}
