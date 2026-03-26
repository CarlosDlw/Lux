#pragma once

#include <string>
#include <vector>

#include "helpc/HelpCData.h"

// Parses a C header using libclang and extracts extended information
// (parameter names, doc comments, type sizes, field offsets) for
// display by the helpc command.
class HelpCParser {
public:
    explicit HelpCParser(const std::vector<std::string>& includePaths = {});

    // Parse the given header and return all extracted symbols.
    // headerName can be "raylib.h", "stdio.h", "curl/curl.h", etc.
    // Returns false on parse failure.
    bool parse(const std::string& headerName, HelpCHeaderInfo& out);

private:
    std::vector<std::string> includePaths_;
    std::vector<std::string> systemIncludes_;

    void discoverSystemIncludes();

    // Resolve a short name like "raylib" to "raylib.h", "SDL2" to "SDL2/SDL.h", etc.
    static std::string resolveShortName(const std::string& input);
};
