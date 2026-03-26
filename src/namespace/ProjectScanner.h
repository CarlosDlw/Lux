#pragma once

#include <string>
#include <vector>

// Recursively scans a project directory for .lx source files.
// Skips hidden directories (starting with '.') and the .luxbuild directory.
class ProjectScanner {
public:
    static std::vector<std::string> scan(const std::string& rootDir);
};
