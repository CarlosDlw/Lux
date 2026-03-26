#pragma once

#include <string>
#include <vector>
#include <memory>

#include "parser/Parser.h"
#include "namespace/NamespaceRegistry.h"

struct CLIOptions {
    std::string inputFile;
    std::string outputFile;
    int         optimizationLevel = 0;
    bool        showIR      = false;
    bool        showHelp    = false;
    bool        showVersion = false;

    std::vector<std::string> linkerFlags;   // -lxxx
    std::vector<std::string> libPaths;      // -Lpath
    std::vector<std::string> includePaths;  // -Ipath
};

// Holds a parsed source file together with its namespace.
struct SourceUnit {
    std::string filePath;
    std::string namespaceName;
    ParseResult parseResult;
};

class CLI {
public:
    CLI(int argc, char* argv[]);
    int run();

private:
    CLIOptions options_;
    int    argc_ = 0;
    char** argv_ = nullptr;
    bool   isHelpC_ = false;

    bool parse(int argc, char* argv[]);
    void printHelp()    const;
    void printVersion() const;
    int  compile();

    // Multi-file pipeline helpers
    std::string getProjectRoot() const;
    std::string ensureBuildDir(const std::string& projectRoot) const;
    static std::string extractNamespace(ToLLVMParser::ProgramContext* tree);
    static std::string makeObjectName(const SourceUnit& unit);
};
