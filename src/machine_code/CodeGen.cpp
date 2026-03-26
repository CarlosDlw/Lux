#include "machine_code/CodeGen.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>

// getDefaultTargetTriple / getHostCPUName moved in LLVM 15
#ifdef LLVM_VERSION_15_OR_NEWER
#  include <llvm/TargetParser/Host.h>
#else
#  include <llvm/Support/Host.h>
#endif

// CodeGenFileType enum was renamed in LLVM 18
#ifdef LLVM_VERSION_18_OR_NEWER
#  define TOLLVM_CGFT_OBJECT llvm::CodeGenFileType::ObjectFile
#else
#  define TOLLVM_CGFT_OBJECT llvm::CGFT_ObjectFile
#endif

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

// ── Internal helpers ─────────────────────────────────────────────────────────

// Spawn `linker objectPath builtinsPath -o outputPath` and wait for it.
// Returns true only if the child exits with code 0.
static bool tryLink(const char*        linker,
                    const std::string& objectPath,
                    const std::string& builtinsPath,
                    const std::string& outputPath) {
    pid_t pid = ::fork();
    if (pid < 0) return false;

    if (pid == 0) {
        // Child process
        const char* args[] = {
            linker,
            objectPath.c_str(),
            builtinsPath.c_str(),
            "-lm",
            "-lz",
            "-lpthread",
            "-o", outputPath.c_str(),
            nullptr
        };
        ::execvp(linker, const_cast<char**>(args));
        ::_exit(127); // exec failed
    }

    int status = 0;
    ::waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

// Spawn a linker with a list of object files + builtins.
static bool tryLinkMulti(const char*                      linker,
                          const std::vector<std::string>& objectPaths,
                          const std::string&              builtinsPath,
                          const std::string&              outputPath,
                          const std::vector<std::string>& extraLinkerFlags,
                          const std::vector<std::string>& extraLibPaths) {
    pid_t pid = ::fork();
    if (pid < 0) return false;

    if (pid == 0) {
        // Build argv: linker obj1 obj2 ... builtins -Lpath... -lm -lz -lpthread -lfoo... -o out
        std::vector<const char*> argv;
        argv.push_back(linker);
        for (auto& obj : objectPaths)
            argv.push_back(obj.c_str());
        argv.push_back(builtinsPath.c_str());
        for (auto& lp : extraLibPaths)
            argv.push_back(lp.c_str());
        argv.push_back("-lm");
        argv.push_back("-lz");
        argv.push_back("-lpthread");
        for (auto& lf : extraLinkerFlags)
            argv.push_back(lf.c_str());
        argv.push_back("-o");
        argv.push_back(outputPath.c_str());
        argv.push_back(nullptr);
        ::execvp(linker, const_cast<char**>(argv.data()));
        ::_exit(127);
    }

    int status = 0;
    ::waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

// Locate the builtins static library next to the tollvm executable.
static std::string findBuiltinsPath() {
    char selfPath[4096];
    ssize_t len = ::readlink("/proc/self/exe", selfPath, sizeof(selfPath) - 1);
    if (len > 0) {
        selfPath[len] = '\0';
        std::string dir(selfPath);
        dir = dir.substr(0, dir.rfind('/'));
        return dir + "/libtollvm_builtins.a";
    }
    return "libtollvm_builtins.a"; // fallback
}

// ── Public API ───────────────────────────────────────────────────────────────

bool CodeGen::compileCSource(const std::string& cSourcePath,
                              const std::string& objectPath,
                              const std::vector<std::string>& extraIncludePaths) {
    const char* compilers[] = { "cc", "clang", "gcc" };

    for (auto* cc : compilers) {
        pid_t pid = ::fork();
        if (pid < 0) return false;

        if (pid == 0) {
            std::vector<const char*> argv;
            argv.push_back(cc);
            argv.push_back("-c");
            argv.push_back(cSourcePath.c_str());

            for (auto& ip : extraIncludePaths)
                argv.push_back(ip.c_str());

            argv.push_back("-o");
            argv.push_back(objectPath.c_str());
            argv.push_back(nullptr);

            ::execvp(cc, const_cast<char**>(argv.data()));
            ::_exit(127);
        }

        int status = 0;
        ::waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            return true;
    }

    std::cerr << "tollvm: failed to compile C source '"
              << cSourcePath << "' — ensure cc, clang or gcc is installed\n";
    return false;
}

bool CodeGen::emitBinary(IRModule& irModule, const std::string& outputPath) {
    const std::string objectPath = outputPath + ".o";

    if (!emitObjectFile(irModule.module(), objectPath)) {
        return false;
    }

    auto builtinsPath = findBuiltinsPath();

    bool linked = tryLink("clang", objectPath, builtinsPath, outputPath)
               || tryLink("gcc",   objectPath, builtinsPath, outputPath);

    llvm::sys::fs::remove(objectPath);

    if (!linked) {
        std::cerr << "tollvm: linking failed — ensure clang or gcc is installed\n";
    }
    return linked;
}

bool CodeGen::linkObjectFiles(const std::vector<std::string>& objectPaths,
                               const std::string& outputPath,
                               const std::vector<std::string>& extraLinkerFlags,
                               const std::vector<std::string>& extraLibPaths) {
    auto builtinsPath = findBuiltinsPath();

    bool linked = tryLinkMulti("clang", objectPaths, builtinsPath, outputPath,
                               extraLinkerFlags, extraLibPaths)
               || tryLinkMulti("gcc",   objectPaths, builtinsPath, outputPath,
                               extraLinkerFlags, extraLibPaths);

    if (!linked) {
        std::cerr << "tollvm: linking failed — ensure clang or gcc is installed\n";
    }
    return linked;
}

// ── Private helpers ──────────────────────────────────────────────────────────

bool CodeGen::emitObjectFile(llvm::Module* module, const std::string& objectPath) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    llvm::Triple targetTriple(llvm::sys::getDefaultTargetTriple());
    module->setTargetTriple(targetTriple);

    std::string lookupError;
    const auto* target = llvm::TargetRegistry::lookupTarget(targetTriple, lookupError);
    if (!target) {
        std::cerr << "tollvm: target lookup failed: " << lookupError << "\n";
        return false;
    }

    auto cpu      = llvm::sys::getHostCPUName();
    auto features = llvm::StringRef("");

    llvm::TargetOptions opt;
    std::unique_ptr<llvm::TargetMachine> machine(
        target->createTargetMachine(targetTriple, cpu, features, opt, llvm::Reloc::PIC_));

    module->setDataLayout(machine->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(objectPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << "tollvm: could not open '" << objectPath << "': "
                  << ec.message() << "\n";
        return false;
    }

    llvm::legacy::PassManager passManager;
    if (machine->addPassesToEmitFile(passManager, dest, nullptr, TOLLVM_CGFT_OBJECT)) {
        std::cerr << "tollvm: target machine cannot emit object files\n";
        return false;
    }

    passManager.run(*module);
    dest.flush();
    return true;
}
