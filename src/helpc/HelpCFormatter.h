#pragma once

#include <string>
#include <vector>

#include "helpc/HelpCData.h"

struct HelpCOptions {
    bool showAll     = false;  // --all: don't truncate lists
    bool verbose     = false;  // --verbose: show offsets, sizes, raw C decl
    bool showRelated = false;  // --related: show related symbols
    bool jsonOutput  = false;  // --json: machine-readable output
    bool noColor     = false;  // --no-color: disable ANSI colors
    bool useRegex    = false;  // --regex: treat search as regex
};

class HelpCFormatter {
public:
    explicit HelpCFormatter(const HelpCOptions& opts);

    // ── Single symbol display ───────────────────────────────────────────
    void printFunction(const HelpCFunction& fn, const HelpCHeaderInfo& hdr) const;
    void printStruct(const HelpCStruct& s, const HelpCHeaderInfo& hdr) const;
    void printEnum(const HelpCEnum& e, const HelpCHeaderInfo& hdr) const;
    void printTypedef(const HelpCTypedef& td, const HelpCHeaderInfo& hdr) const;
    void printMacro(const HelpCMacro& m, const HelpCHeaderInfo& hdr) const;
    void printStructMacro(const HelpCStructMacro& sm, const HelpCHeaderInfo& hdr) const;
    void printGlobal(const HelpCGlobal& g, const HelpCHeaderInfo& hdr) const;

    // ── Listing ─────────────────────────────────────────────────────────
    void printSummary(const HelpCHeaderInfo& hdr) const;
    void printListFunctions(const HelpCHeaderInfo& hdr) const;
    void printListStructs(const HelpCHeaderInfo& hdr) const;
    void printListEnums(const HelpCHeaderInfo& hdr) const;
    void printListMacros(const HelpCHeaderInfo& hdr) const;
    void printListTypedefs(const HelpCHeaderInfo& hdr) const;
    void printListGlobals(const HelpCHeaderInfo& hdr) const;

    // ── Search results ──────────────────────────────────────────────────
    struct SearchResult {
        std::string name;
        std::string kind;       // "function", "struct", "enum", etc.
        std::string summary;    // short one-line description
    };
    void printSearchResults(const std::vector<SearchResult>& results,
                            const std::string& query,
                            const HelpCHeaderInfo& hdr) const;

    // ── Related symbols ─────────────────────────────────────────────────
    void printRelatedFunctions(const std::string& typeName,
                               const HelpCHeaderInfo& hdr) const;
    void printRelatedMacros(const std::string& typeName,
                            const HelpCHeaderInfo& hdr) const;

    // ── JSON output ─────────────────────────────────────────────────────
    void printFunctionJson(const HelpCFunction& fn, const HelpCHeaderInfo& hdr) const;
    void printStructJson(const HelpCStruct& s, const HelpCHeaderInfo& hdr) const;
    void printEnumJson(const HelpCEnum& e, const HelpCHeaderInfo& hdr) const;
    void printHeaderJson(const HelpCHeaderInfo& hdr) const;

private:
    HelpCOptions opts_;

    // ANSI color helpers (no-ops if noColor)
    std::string bold(const std::string& s) const;
    std::string dim(const std::string& s) const;
    std::string cyan(const std::string& s) const;
    std::string green(const std::string& s) const;
    std::string yellow(const std::string& s) const;
    std::string magenta(const std::string& s) const;
    std::string red(const std::string& s) const;
    std::string blue(const std::string& s) const;
    std::string reset() const;

    // Box drawing
    void printBoxTop(const std::string& label, int width = 60) const;
    void printBoxLine(const std::string& content, int width = 60) const;
    void printBoxBottom(int width = 60) const;
    void printBoxEmpty(int width = 60) const;

    // Utilities
    static std::string padRight(const std::string& s, size_t width);
    static std::string formatHex(int64_t value);
    std::string functionSignature(const HelpCFunction& fn) const;
};
