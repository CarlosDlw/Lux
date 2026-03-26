#pragma once

#include <string>
#include <vector>

#include "helpc/HelpCData.h"
#include "helpc/HelpCFormatter.h"

// Command-line options for the helpc subcommand.
struct HelpCCommand {
    std::string header;         // library/header name (required)
    std::string symbol;         // symbol to look up (optional)
    std::string searchQuery;    // --search query
    std::string listCategory;   // --list [category]
    HelpCOptions displayOpts;   // formatting flags
    std::vector<std::string> includePaths;  // -I paths
};

// Orchestrator for the helpc subcommand.
// Parses the header, looks up the symbol, and prints the result.
class HelpC {
public:
    // Parse argc/argv starting AFTER "helpc" (i.e. argv[2], argv[3], ...).
    // Returns false if args are invalid.
    static bool parseArgs(int argc, char* argv[], HelpCCommand& cmd);

    // Execute the helpc command. Returns exit code.
    static int run(const HelpCCommand& cmd);

    // Print helpc usage.
    static void printUsage();

private:
    // Look up a single symbol by exact name.
    // Returns true if found and printed.
    static bool lookupSymbol(const std::string& name,
                             const HelpCHeaderInfo& hdr,
                             const HelpCFormatter& fmt,
                             const HelpCOptions& opts);
};
