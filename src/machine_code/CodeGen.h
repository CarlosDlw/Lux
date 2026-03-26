#pragma once

#include <string>
#include <vector>
#include "LLVM_IR/IRModule.h"

class CodeGen {
public:
    // Emit a native executable from a single module (legacy single-file path).
    static bool emitBinary(IRModule& irModule, const std::string& outputPath);

    // Emit a single LLVM module as an object file.
    static bool emitObjectFile(llvm::Module* module, const std::string& objectPath);

    // Link multiple object files into a single executable.
    static bool linkObjectFiles(const std::vector<std::string>& objectPaths,
                                const std::string& outputPath,
                                const std::vector<std::string>& extraLinkerFlags = {},
                                const std::vector<std::string>& extraLibPaths = {});

    // Compile a C source file into an object file using the system C compiler.
    static bool compileCSource(const std::string& cSourcePath,
                               const std::string& objectPath,
                               const std::vector<std::string>& extraIncludePaths = {});

private:
    static bool linkToExecutable(const std::string& objectPath,
                                 const std::string& outputPath);
};
