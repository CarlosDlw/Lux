#include "cli/CLI.h"
#include "parser/Parser.h"
#include "checkers/Checker.h"
#include "IRBuilder/IRGen.h"
#include "LLVM_IR/IRModule.h"
#include "LLVM_Optimizer/Optimizer.h"
#include "machine_code/CodeGen.h"
#include "namespace/ProjectScanner.h"
#include "namespace/NamespaceRegistry.h"
#include "ffi/CBindings.h"
#include "ffi/CHeaderResolver.h"
#include "helpc/HelpC.h"
#include "lsp/LspServer.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>

#ifdef LUX_RUNTIME_DIAGNOSTICS
#include <array>
#include <csignal>
#include <cstring>
#include <execinfo.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

#ifdef LUX_RUNTIME_DIAGNOSTICS
namespace {

volatile sig_atomic_t g_inCrashHandler = 0;

const char* signalName(int sig) {
    switch (sig) {
        case SIGSEGV: return "SIGSEGV";
        case SIGABRT: return "SIGABRT";
#ifdef SIGBUS
        case SIGBUS:  return "SIGBUS";
#endif
        case SIGFPE:  return "SIGFPE";
        case SIGILL:  return "SIGILL";
        default:      return "SIGNAL";
    }
}

const char* probableCause(int sig) {
    switch (sig) {
        case SIGSEGV:
#ifdef SIGBUS
        case SIGBUS:
#endif
            return "invalid memory access (overflow/use-after-free/null dereference)";
        case SIGABRT:
            return "detected memory corruption or explicit abort()";
        case SIGFPE:
            return "invalid arithmetic operation (division by zero or overflow trap)";
        case SIGILL:
            return "invalid instruction or corrupted control flow";
        default:
            return "runtime fatal condition";
    }
}

void runtimeCrashHandler(int sig, siginfo_t* info, void*) {
    if (g_inCrashHandler) _Exit(128 + sig);
    g_inCrashHandler = 1;

    std::string msg;
    msg += "\nlux: runtime crash detected in JIT execution\n";
    msg += "lux: signal: ";
    msg += signalName(sig);
    msg += " (" + std::to_string(sig) + ")\n";

    if (info) {
        msg += "lux: fault address: ";
        msg += std::to_string(reinterpret_cast<uintptr_t>(info->si_addr));
        msg += "\n";
    }

    msg += "lux: probable cause: ";
    msg += probableCause(sig);
    msg += "\n";
    msg += "lux: stack trace:\n";
    (void)!write(STDERR_FILENO, msg.c_str(), msg.size());

    void* frames[64];
    int n = backtrace(frames, 64);
    if (n > 0)
        backtrace_symbols_fd(frames, n, STDERR_FILENO);

    const char* hint =
        "lux: hint: rebuild with -DCMAKE_BUILD_TYPE=Debug to maximize diagnostics\n";
    (void)!write(STDERR_FILENO, hint, std::strlen(hint));

    _Exit(128 + sig);
}

class RuntimeSignalGuard {
public:
    RuntimeSignalGuard() {
        install(SIGSEGV, idx_++);
        install(SIGABRT, idx_++);
#ifdef SIGBUS
        install(SIGBUS, idx_++);
#endif
        install(SIGFPE, idx_++);
        install(SIGILL, idx_++);
    }

    ~RuntimeSignalGuard() {
        for (int i = 0; i < idx_; i++)
            sigaction(installed_[i], &oldActions_[i], nullptr);
    }

private:
    void install(int sig, int slot) {
        installed_[slot] = sig;
        struct sigaction sa;
        std::memset(&sa, 0, sizeof(sa));
        sa.sa_sigaction = runtimeCrashHandler;
        sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
        sigemptyset(&sa.sa_mask);
        sigaction(sig, &sa, &oldActions_[slot]);
    }

    std::array<struct sigaction, 8> oldActions_{};
    std::array<int, 8> installed_{};
    int idx_ = 0;
};

}
#endif

CLI::CLI(int argc, char* argv[])
    : argc_(argc), argv_(argv)
{
    // Check for subcommands early
    if (argc >= 2 && std::string(argv[1]) == "helpc") {
        isHelpC_ = true;
        return;
    }
    if (argc >= 2 && std::string(argv[1]) == "lsp") {
        isLSP_ = true;
        return;
    }
    if (argc >= 3 && std::string(argv[1]) == "run") {
        isRun_ = true;
        options_.runJIT   = true;
        options_.inputFile = argv[2];
        // Collect -l/-L/-I flags; everything after -- goes to runArgs
        bool doubleDash = false;
        for (int i = 3; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--") { doubleDash = true; continue; }
            if (doubleDash) {
                options_.runArgs.push_back(arg);
            } else if (arg.rfind("-l", 0) == 0 && arg.size() > 2) {
                options_.linkerFlags.push_back(arg);
            } else if (arg.rfind("-L", 0) == 0 && arg.size() > 2) {
                options_.libPaths.push_back(arg);
            } else if (arg.rfind("-I", 0) == 0 && arg.size() > 2) {
                options_.includePaths.push_back(arg);
            }
        }
        return;
    }
    if (!parse(argc, argv)) {
        options_.showHelp = true;
    }
}

bool CLI::parse(int argc, char* argv[]) {
    if (argc < 2) {
        return false;
    }

    std::string first = argv[1];

    if (first == "help" || first == "--help" || first == "-h") {
        options_.showHelp = true;
        return true;
    }

    if (first == "version" || first == "--version" || first == "-v") {
        options_.showVersion = true;
        return true;
    }

    options_.inputFile = first;

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-o1") {
            options_.optimizationLevel = 1;
        } else if (arg == "-o2") {
            options_.optimizationLevel = 2;
        } else if (arg == "-o3") {
            options_.optimizationLevel = 3;
        } else if (arg.rfind("-l", 0) == 0 && arg.size() > 2) {
            options_.linkerFlags.push_back(arg);
        } else if (arg.rfind("-L", 0) == 0 && arg.size() > 2) {
            options_.libPaths.push_back(arg);
        } else if (arg.rfind("-I", 0) == 0 && arg.size() > 2) {
            options_.includePaths.push_back(arg);
        } else if (arg[0] != '-') {
            options_.outputFile = arg;
        } else {
            std::cerr << "lux: unknown flag '" << arg << "'\n";
            std::cerr << "Run 'lux help' for usage.\n";
            return false;
        }
    }

    options_.showIR = options_.outputFile.empty();
    return true;
}

void CLI::printHelp() const {
    std::cout
        << "lux - Lux Compiler  v0.1.0\n\n"
        << "Usage:\n"
        << "  lux <file.lx>              Compile and print LLVM IR to stdout\n"
        << "  lux <file.lx> <output>     Compile and emit native binary\n"
        << "  lux <file.lx> <output> -oN Compile with optimization level N (1, 2, or 3)\n"
        << "  lux run <file.lx>          JIT-execute via LLVM IR (no binary emitted)\n"
        << "  lux run <file.lx> -- ...   JIT-execute, forwarding args to main\n"
        << "  lux helpc <lib> [symbol]   C library reference helper\n"
        << "  lux lsp                    Start LSP server (for editor integration)\n"
        << "  lux help                   Show this help message\n"
        << "  lux version                Show compiler version\n\n"
        << "Linker flags:\n"
        << "  -lname       Link against library 'name' (e.g. -lSDL2)\n"
        << "  -Lpath       Add 'path' to the library search path\n"
        << "  -Ipath       Add 'path' to the include search path (reserved)\n\n"
        << "Examples:\n"
        << "  lux main.lx\n"
        << "  lux main.lx ./main\n"
        << "  lux main.lx ./main -o2\n"
        << "  lux run main.lx\n"
        << "  lux run main.lx -lraylib\n";
}

void CLI::printVersion() const {
    std::cout << "lux v0.1.0\n";
}

int CLI::run() {
    if (isLSP_) {
        LspServer server;
        return server.run();
    }
    if (isHelpC_) {
        HelpCCommand cmd;
        if (!HelpC::parseArgs(argc_, argv_, cmd)) {
            HelpC::printUsage();
            return 1;
        }
        return HelpC::run(cmd);
    }
    if (isRun_) {
        return jitRun();
    }
    if (options_.showHelp) {
        printHelp();
        return 0;
    }
    if (options_.showVersion) {
        printVersion();
        return 0;
    }
    return compile();
}

// ── Helpers ──────────────────────────────────────────────────────────────────

std::string CLI::getProjectRoot() const {
    auto parent = fs::path(options_.inputFile).parent_path();
    if (parent.empty()) parent = ".";
    return fs::canonical(parent).string();
}

std::string CLI::ensureBuildDir(const std::string& projectRoot) const {
    auto buildDir = projectRoot + "/.luxbuild";
    fs::create_directories(buildDir);
    return buildDir;
}

std::string CLI::extractNamespace(LuxParser::ProgramContext* tree) {
    if (auto* nsDecl = tree->namespaceDecl()) {
        return nsDecl->IDENTIFIER()->getText();
    }
    return {};
}

std::string CLI::makeObjectName(const SourceUnit& unit) {
    // Use namespace + sanitized filename stem for uniqueness.
    auto stem = fs::path(unit.filePath).stem().string();
    return unit.namespaceName + "__" + stem;
}

// ── Main compilation pipeline ────────────────────────────────────────────────

int CLI::compile() {
    // ═════════════════════════════════════════════════════════════════════
    // STEP 1: DETERMINE PROJECT ROOT & BUILD DIRECTORY
    // ═════════════════════════════════════════════════════════════════════
    auto projectRoot = getProjectRoot();
    auto buildDir    = ensureBuildDir(projectRoot);

    // ═════════════════════════════════════════════════════════════════════
    // STEP 2: SCAN ALL .lx FILES IN THE PROJECT
    // ═════════════════════════════════════════════════════════════════════
    auto allFiles = ProjectScanner::scan(projectRoot);
    if (allFiles.empty()) {
        std::cerr << "lux: no .lx files found in '" << projectRoot << "'\n";
        return 1;
    }

    // ═════════════════════════════════════════════════════════════════════
    // STEP 3: PARSE ALL FILES
    // ═════════════════════════════════════════════════════════════════════
    std::vector<SourceUnit> units;
    bool anyParseError = false;

    for (auto& filePath : allFiles) {
        SourceUnit unit;
        unit.filePath    = filePath;
        unit.parseResult = Parser::parse(filePath);

        if (unit.parseResult.hasErrors) {
            std::cerr << "lux: parse errors in '" << filePath << "'\n";
            anyParseError = true;
            continue;
        }

        unit.namespaceName = extractNamespace(unit.parseResult.tree);
        if (unit.namespaceName.empty()) {
            std::cerr << "lux: file '" << filePath
                      << "' is missing a 'namespace' declaration\n";
            anyParseError = true;
            continue;
        }

        units.push_back(std::move(unit));
    }

    if (anyParseError) return 1;

    // ═════════════════════════════════════════════════════════════════════
    // STEP 4: BUILD NAMESPACE REGISTRY
    // ═════════════════════════════════════════════════════════════════════
    NamespaceRegistry registry;
    for (auto& unit : units) {
        registry.registerFile(unit.namespaceName, unit.filePath,
                              unit.parseResult.tree);
    }

    // Check for duplicate symbols within namespaces
    auto dupErrors = registry.validate();
    if (!dupErrors.empty()) {
        for (auto& err : dupErrors) {
            std::cerr << "lux: " << err << "\n";
        }
        return 1;
    }

    // ═════════════════════════════════════════════════════════════════════
    // STEP 4.5: RESOLVE C HEADER INCLUDES
    // ═════════════════════════════════════════════════════════════════════
    CBindings cBindings;
    TypeRegistry cTypeReg;
    std::vector<std::string> cSourceFiles;  // C sources to compile
    {
        CHeaderResolver resolver(cTypeReg, cBindings, options_.includePaths);

        for (auto& unit : units) {
            auto* tree = unit.parseResult.tree;
            for (auto* pre : tree->preambleDecl()) {
                auto* incl = pre->includeDecl();
                if (!incl) continue;
                auto text = incl->getText();
                if (incl->INCLUDE_SYS()) {
                    auto header = CHeaderResolver::extractSystemHeader(text);
                    if (header.empty()) {
                        std::cerr << "lux: invalid system include in '"
                                  << unit.filePath << "'\n";
                        continue;
                    }
                    if (!resolver.resolveSystemHeader(header)) {
                        std::cerr << "lux: cannot find header '<" << header
                                  << ">'. Check '-I' include paths\n";
                        return 1;
                    }
                } else if (incl->INCLUDE_LOCAL()) {
                    auto header = CHeaderResolver::extractLocalHeader(text);
                    if (header.empty()) {
                        std::cerr << "lux: invalid local include in '"
                                  << unit.filePath << "'\n";
                        continue;
                    }
                    auto basePath = fs::path(unit.filePath).parent_path().string();
                    if (!resolver.resolveLocalHeader(header, basePath)) {
                        std::cerr << "lux: cannot find header '\"" << header
                                  << "\"'. Check '-I' include paths\n";
                        return 1;
                    }

                    // Check for a corresponding .c source file
                    auto headerPath = fs::path(basePath) / header;
                    auto cPath = fs::path(headerPath).replace_extension(".c");
                    if (fs::exists(cPath)) {
                        auto canonical = fs::canonical(cPath).string();
                        if (std::find(cSourceFiles.begin(), cSourceFiles.end(),
                                      canonical) == cSourceFiles.end()) {
                            cSourceFiles.push_back(canonical);
                        }
                    }
                }
            }
        }
    }

    // ═════════════════════════════════════════════════════════════════════
    // STEP 4.5b: AUTO-DETECT REQUIRED LINKER LIBRARIES
    // ═════════════════════════════════════════════════════════════════════
    for (auto& [flag, header] : cBindings.requiredLibs()) {
        // Check if the user already provided this flag
        bool alreadyProvided = false;
        for (auto& lf : options_.linkerFlags) {
            if (lf == flag) { alreadyProvided = true; break; }
        }
        if (!alreadyProvided) {
            std::cerr << "lux: auto-linking '" << flag
                      << "' (required by <" << header << ">)\n";
            options_.linkerFlags.push_back(flag);
        }
    }

    // ═════════════════════════════════════════════════════════════════════
    // STEP 4.6: COMPILE LOCAL C SOURCES
    // ═════════════════════════════════════════════════════════════════════
    std::vector<std::string> cObjectFiles;
    for (auto& cSrc : cSourceFiles) {
        auto stem = fs::path(cSrc).stem().string();
        auto objPath = buildDir + "/c__" + stem + ".o";

        // Build -I flags for the C compiler
        std::vector<std::string> cIncFlags;
        cIncFlags.push_back("-I" + fs::path(cSrc).parent_path().string());
        for (auto& ip : options_.includePaths) {
            if (ip.rfind("-I", 0) == 0)
                cIncFlags.push_back(ip);
            else
                cIncFlags.push_back("-I" + ip);
        }

        if (!CodeGen::compileCSource(cSrc, objPath, cIncFlags)) {
            std::cerr << "lux: failed to compile C source '"
                      << cSrc << "'\n";
            return 1;
        }
        cObjectFiles.push_back(objPath);
    }

    // ═════════════════════════════════════════════════════════════════════
    // STEP 5: SEMANTIC CHECK ALL FILES
    // ═════════════════════════════════════════════════════════════════════
    bool anyCheckError = false;
    for (auto& unit : units) {
        Checker checker;
        checker.setNamespaceContext(&registry, unit.namespaceName, unit.filePath);
        checker.setCBindings(&cBindings);
        bool passed = checker.check(unit.parseResult.tree);
        // Always print diagnostics (errors and warnings)
        for (auto& err : checker.errors()) {
            std::cerr << "lux: " << unit.filePath << ": " << err << "\n";
        }
        if (!passed)
            anyCheckError = true;
    }
    if (anyCheckError) return 1;

    // ═════════════════════════════════════════════════════════════════════
    // STEP 6: GENERATE IR, OPTIMIZE, AND EMIT OBJECT FILES
    // ═════════════════════════════════════════════════════════════════════
    std::vector<std::string> objectFiles;
    bool anyIRError = false;

    for (auto& unit : units) {
        IRGen irGen;
        irGen.setNamespaceContext(&registry, unit.namespaceName, unit.filePath);
        irGen.setCBindings(&cBindings);
        auto irModule = irGen.generate(unit.parseResult.tree, unit.filePath);
        if (!irModule) {
            std::cerr << "lux: IR generation failed for '"
                      << unit.filePath << "'\n";
            anyIRError = true;
            continue;
        }

        // Show IR mode: only print the main file's IR
        if (options_.showIR) {
            auto mainPath = fs::canonical(fs::path(options_.inputFile)).string();
            if (unit.filePath == mainPath) {
                irModule->print();
            }
            continue;
        }

        // Optimize
        if (options_.optimizationLevel > 0) {
            auto level = static_cast<OptimizationLevel>(options_.optimizationLevel);
            Optimizer::optimize(*irModule, level);
        }

        // Emit object file to .luxbuild/
        auto objName = makeObjectName(unit);
        auto objPath = buildDir + "/" + objName + ".o";
        if (!CodeGen::emitObjectFile(irModule->module(), objPath)) {
            std::cerr << "lux: failed to emit object for '"
                      << unit.filePath << "'\n";
            anyIRError = true;
            continue;
        }
        objectFiles.push_back(objPath);
    }

    if (anyIRError) return 1;
    if (options_.showIR) return 0;

    // Append compiled C object files
    objectFiles.insert(objectFiles.end(),
                       cObjectFiles.begin(), cObjectFiles.end());

    // ═════════════════════════════════════════════════════════════════════
    // STEP 7: LINK ALL OBJECT FILES
    // ═════════════════════════════════════════════════════════════════════
    if (!CodeGen::linkObjectFiles(objectFiles, options_.outputFile,
                                    options_.linkerFlags, options_.libPaths)) {
        std::cerr << "lux: failed to link binary '"
                  << options_.outputFile << "'\n";
        return 1;
    }

    std::cout << "lux: binary written to '" << options_.outputFile << "'\n";
    return 0;
}

// ── JIT Runner ───────────────────────────────────────────────────────────────
//
// Runs the full compile pipeline (parse → check → IRGen for all project files),
// links all generated modules into one via bitcode serialization (so they can
// cross-context merge), then executes main() in-process via LLVM MCJIT.
//
// External symbols (libc, etc.) are resolved from the host process image.
// Shared libraries specified with -l flags are dlopen'd before JIT finalization.

int CLI::jitRun() {
#ifdef LUX_RUNTIME_DIAGNOSTICS
    RuntimeSignalGuard signalGuard;
#endif

    // ─── Steps 1-5: same as compile() ────────────────────────────────────
    auto projectRoot = getProjectRoot();

    auto allFiles = ProjectScanner::scan(projectRoot);
    if (allFiles.empty()) {
        std::cerr << "lux: no .lx files found in '" << projectRoot << "'\n";
        return 1;
    }

    std::vector<SourceUnit> units;
    bool anyParseError = false;
    for (auto& filePath : allFiles) {
        SourceUnit unit;
        unit.filePath    = filePath;
        unit.parseResult = Parser::parse(filePath);
        if (unit.parseResult.hasErrors) {
            std::cerr << "lux: parse errors in '" << filePath << "'\n";
            anyParseError = true;
            continue;
        }
        unit.namespaceName = extractNamespace(unit.parseResult.tree);
        if (unit.namespaceName.empty()) {
            std::cerr << "lux: file '" << filePath
                      << "' is missing a 'namespace' declaration\n";
            anyParseError = true;
            continue;
        }
        units.push_back(std::move(unit));
    }
    if (anyParseError) return 1;

    NamespaceRegistry registry;
    for (auto& unit : units)
        registry.registerFile(unit.namespaceName, unit.filePath,
                              unit.parseResult.tree);

    auto dupErrors = registry.validate();
    if (!dupErrors.empty()) {
        for (auto& err : dupErrors) std::cerr << "lux: " << err << "\n";
        return 1;
    }

    CBindings cBindings;
    TypeRegistry cTypeReg;
    {
        CHeaderResolver resolver(cTypeReg, cBindings, options_.includePaths);
        for (auto& unit : units) {
            for (auto* pre : unit.parseResult.tree->preambleDecl()) {
                auto* incl = pre->includeDecl();
                if (!incl) continue;
                auto text = incl->getText();
                if (incl->INCLUDE_SYS()) {
                    auto header = CHeaderResolver::extractSystemHeader(text);
                    if (!header.empty())
                        resolver.resolveSystemHeader(header);
                } else if (incl->INCLUDE_LOCAL()) {
                    auto header = CHeaderResolver::extractLocalHeader(text);
                    if (!header.empty()) {
                        auto base = fs::path(unit.filePath).parent_path().string();
                        resolver.resolveLocalHeader(header, base);
                    }
                }
            }
        }
    }

    // Auto-link libraries required by C headers
    for (auto& [flag, header] : cBindings.requiredLibs()) {
        bool already = false;
        for (auto& lf : options_.linkerFlags) if (lf == flag) { already = true; break; }
        if (!already) {
            std::cerr << "lux: auto-linking '" << flag
                      << "' (required by <" << header << ">)\n";
            options_.linkerFlags.push_back(flag);
        }
    }

    bool anyCheckError = false;
    for (auto& unit : units) {
        Checker checker;
        checker.setNamespaceContext(&registry, unit.namespaceName, unit.filePath);
        checker.setCBindings(&cBindings);
        bool passed = checker.check(unit.parseResult.tree);
        for (auto& err : checker.errors())
            std::cerr << "lux: " << unit.filePath << ": " << err << "\n";
        if (!passed) anyCheckError = true;
    }
    if (anyCheckError) return 1;

    // ─── Step 6: Generate IR for each unit, link into one module ─────────

    // We use the first unit's context as the master context.
    // Subsequent modules are serialized to bitcode and re-parsed into it,
    // allowing llvm::Linker to merge modules from different IRGen contexts.

    std::unique_ptr<llvm::LLVMContext> masterCtx;
    std::unique_ptr<llvm::Module>      masterMod;

    for (auto& unit : units) {
        IRGen irGen;
        irGen.setNamespaceContext(&registry, unit.namespaceName, unit.filePath);
        irGen.setCBindings(&cBindings);
        auto irMod = irGen.generate(unit.parseResult.tree, unit.filePath);
        if (!irMod) {
            std::cerr << "lux: IR generation failed for '"
                      << unit.filePath << "'\n";
            return 1;
        }

        if (!masterMod) {
            // First module: take ownership directly
            masterCtx = std::make_unique<llvm::LLVMContext>();

            // Serialize the generated module to bitcode, then re-parse into
            // masterCtx so every module lives in the same context.
            llvm::SmallVector<char, 0> buf;
            {
                llvm::raw_svector_ostream os(buf);
                llvm::WriteBitcodeToFile(*irMod->module(), os);
            }
            auto memBuf = llvm::MemoryBuffer::getMemBufferCopy(
                llvm::StringRef(buf.data(), buf.size()), unit.filePath);
            auto parsed = llvm::parseBitcodeFile(memBuf->getMemBufferRef(),
                                                  *masterCtx);
            if (!parsed) {
                std::cerr << "lux: bitcode re-parse failed for '"
                          << unit.filePath << "'\n";
                return 1;
            }
            masterMod = std::move(parsed.get());
        } else {
            // Subsequent modules: serialize and merge into masterMod
            llvm::SmallVector<char, 0> buf;
            {
                llvm::raw_svector_ostream os(buf);
                llvm::WriteBitcodeToFile(*irMod->module(), os);
            }
            auto memBuf = llvm::MemoryBuffer::getMemBufferCopy(
                llvm::StringRef(buf.data(), buf.size()), unit.filePath);
            auto parsed = llvm::parseBitcodeFile(memBuf->getMemBufferRef(),
                                                  *masterCtx);
            if (!parsed) {
                std::cerr << "lux: bitcode re-parse failed for '"
                          << unit.filePath << "'\n";
                return 1;
            }
            if (llvm::Linker::linkModules(*masterMod,
                                           std::move(parsed.get()))) {
                std::cerr << "lux: failed to link module '"
                          << unit.filePath << "'\n";
                return 1;
            }
        }
    }

    if (!masterMod) {
        std::cerr << "lux: no IR generated\n";
        return 1;
    }

    // ─── Step 7: JIT execution ────────────────────────────────────────────

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // Expose the host process symbols (libc, libm, etc.)
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

    // Load shared libraries from -l flags
    for (auto& flag : options_.linkerFlags) {
        if (flag.rfind("-l", 0) != 0 || flag.size() <= 2) continue;
        auto libname = flag.substr(2);

        // Try common shared lib naming conventions
        std::vector<std::string> candidates = {
            "lib" + libname + ".so",
            "lib" + libname + ".so.0",
            "lib" + libname + ".dylib",
        };
        // Prepend -L paths
        std::vector<std::string> searchPaths = {""};
        for (auto& lp : options_.libPaths) {
            auto path = (lp.rfind("-L", 0) == 0) ? lp.substr(2) : lp;
            if (!path.empty()) searchPaths.push_back(path + "/");
        }

        bool loaded = false;
        for (auto& prefix : searchPaths) {
            for (auto& cand : candidates) {
                std::string fullPath = prefix + cand;
                if (!llvm::sys::DynamicLibrary::LoadLibraryPermanently(
                        fullPath.c_str())) {
                    loaded = true;
                    break;
                }
            }
            if (loaded) break;
        }
        if (!loaded) {
            std::cerr << "lux: warning: could not load shared library for '"
                      << flag << "' (JIT may fail on unresolved symbols)\n";
        }
    }

    masterMod->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));

    std::string errStr;
    llvm::EngineBuilder builder(std::move(masterMod));
    builder.setEngineKind(llvm::EngineKind::JIT);
    builder.setErrorStr(&errStr);

    std::unique_ptr<llvm::ExecutionEngine> ee(builder.create());
    if (!ee) {
        std::cerr << "lux: failed to create JIT engine: " << errStr << "\n";
        return 1;
    }

    ee->finalizeObject();

    auto* mainFn = ee->FindFunctionNamed("main");
    if (!mainFn) {
        std::cerr << "lux: no 'main' function found in '"
                  << options_.inputFile << "'\n";
        return 1;
    }

    // Build argv for the JIT-executed program
    std::vector<std::string> progArgv = { options_.inputFile };
    for (auto& a : options_.runArgs) progArgv.push_back(a);

    return ee->runFunctionAsMain(mainFn, progArgv, {});
}
