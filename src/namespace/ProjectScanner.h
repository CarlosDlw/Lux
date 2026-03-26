#pragma once

#include <string>
#include <vector>

// Recursively scans a project directory for .tm source files.
// Skips hidden directories (starting with '.') and the .tmbuild directory.
class ProjectScanner {
public:
    static std::vector<std::string> scan(const std::string& rootDir);
};
