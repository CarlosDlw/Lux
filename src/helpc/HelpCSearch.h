#pragma once

#include <string>
#include <vector>

#include "helpc/HelpCData.h"
#include "helpc/HelpCFormatter.h"

// Search and fuzzy matching over parsed header symbols.
class HelpCSearch {
public:
    // Search all symbol names by case-insensitive substring.
    static std::vector<HelpCFormatter::SearchResult>
    search(const HelpCHeaderInfo& hdr, const std::string& query);

    // Search with regex pattern.
    static std::vector<HelpCFormatter::SearchResult>
    searchRegex(const HelpCHeaderInfo& hdr, const std::string& pattern);

    // Find close matches for a symbol name (fuzzy / did-you-mean).
    static std::vector<HelpCFormatter::SearchResult>
    fuzzyMatch(const HelpCHeaderInfo& hdr, const std::string& name,
               size_t maxResults = 5);

private:
    static int levenshteinDistance(const std::string& a, const std::string& b);
    static std::string toLower(const std::string& s);
    static std::string functionSummary(const HelpCFunction& fn);
};
