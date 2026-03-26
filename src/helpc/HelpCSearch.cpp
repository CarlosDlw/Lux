#include "helpc/HelpCSearch.h"

#include <algorithm>
#include <regex>

// ── Utilities ───────────────────────────────────────────────────────────────

std::string HelpCSearch::toLower(const std::string& s) {
    std::string out = s;
    for (auto& c : out) c = static_cast<char>(std::tolower(c));
    return out;
}

int HelpCSearch::levenshteinDistance(const std::string& a, const std::string& b) {
    size_t m = a.size(), n = b.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    for (size_t i = 0; i <= m; i++) dp[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= n; j++) dp[0][j] = static_cast<int>(j);

    for (size_t i = 1; i <= m; i++) {
        for (size_t j = 1; j <= n; j++) {
            int cost = (std::tolower(a[i - 1]) == std::tolower(b[j - 1])) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            });
        }
    }
    return dp[m][n];
}

std::string HelpCSearch::functionSummary(const HelpCFunction& fn) {
    std::string sig = fn.returnCType + " " + fn.name + "(";
    for (size_t i = 0; i < fn.params.size(); i++) {
        if (i > 0) sig += ", ";
        sig += fn.params[i].cType;
    }
    if (fn.isVariadic) {
        if (!fn.params.empty()) sig += ", ";
        sig += "...";
    }
    sig += ")";
    return sig;
}

// ── Case-insensitive substring search ───────────────────────────────────────

std::vector<HelpCFormatter::SearchResult>
HelpCSearch::search(const HelpCHeaderInfo& hdr, const std::string& query) {
    std::vector<HelpCFormatter::SearchResult> results;
    std::string q = toLower(query);

    for (auto& fn : hdr.functions) {
        if (toLower(fn.name).find(q) != std::string::npos) {
            results.push_back({fn.name, "function", functionSummary(fn)});
        }
    }
    for (auto& s : hdr.structs) {
        std::string kind = s.isUnion ? "union" : "struct";
        if (toLower(s.name).find(q) != std::string::npos) {
            std::string summary = kind + " (" + std::to_string(s.fields.size()) + " fields";
            if (s.size >= 0) summary += ", " + std::to_string(s.size) + " bytes";
            summary += ")";
            results.push_back({s.name, kind, summary});
        }
    }
    for (auto& e : hdr.enums) {
        if (toLower(e.name).find(q) != std::string::npos) {
            results.push_back({e.name, "enum",
                std::to_string(e.values.size()) + " values"});
        }
        // Also search enum values
        for (auto& v : e.values) {
            if (toLower(v.name).find(q) != std::string::npos) {
                results.push_back({v.name, "enum value",
                    e.name + " = " + std::to_string(v.value)});
            }
        }
    }
    for (auto& td : hdr.typedefs) {
        if (toLower(td.name).find(q) != std::string::npos) {
            results.push_back({td.name, "typedef", "→ " + td.underlyingCType});
        }
    }
    for (auto& m : hdr.macros) {
        if (toLower(m.name).find(q) != std::string::npos) {
            results.push_back({m.name, "macro",
                m.isNull ? "NULL" : std::to_string(m.value)});
        }
    }
    for (auto& sm : hdr.structMacros) {
        if (toLower(sm.name).find(q) != std::string::npos) {
            std::string vals = sm.structType + " { ";
            for (size_t i = 0; i < sm.fieldValues.size(); i++) {
                if (i > 0) vals += ", ";
                vals += std::to_string(sm.fieldValues[i]);
            }
            vals += " }";
            results.push_back({sm.name, "macro", vals});
        }
    }
    for (auto& g : hdr.globals) {
        if (toLower(g.name).find(q) != std::string::npos) {
            results.push_back({g.name, "global", g.cType});
        }
    }

    return results;
}

// ── Regex search ────────────────────────────────────────────────────────────

std::vector<HelpCFormatter::SearchResult>
HelpCSearch::searchRegex(const HelpCHeaderInfo& hdr, const std::string& pattern) {
    std::vector<HelpCFormatter::SearchResult> results;

    std::regex re;
    try {
        re = std::regex(pattern, std::regex::icase);
    } catch (const std::regex_error&) {
        return results; // Invalid regex — return empty
    }

    for (auto& fn : hdr.functions) {
        if (std::regex_search(fn.name, re))
            results.push_back({fn.name, "function", functionSummary(fn)});
    }
    for (auto& s : hdr.structs) {
        if (std::regex_search(s.name, re))
            results.push_back({s.name, s.isUnion ? "union" : "struct",
                std::to_string(s.fields.size()) + " fields"});
    }
    for (auto& e : hdr.enums) {
        if (std::regex_search(e.name, re))
            results.push_back({e.name, "enum",
                std::to_string(e.values.size()) + " values"});
        for (auto& v : e.values) {
            if (std::regex_search(v.name, re))
                results.push_back({v.name, "enum value",
                    e.name + " = " + std::to_string(v.value)});
        }
    }
    for (auto& td : hdr.typedefs) {
        if (std::regex_search(td.name, re))
            results.push_back({td.name, "typedef", "→ " + td.underlyingCType});
    }
    for (auto& m : hdr.macros) {
        if (std::regex_search(m.name, re))
            results.push_back({m.name, "macro",
                m.isNull ? "NULL" : std::to_string(m.value)});
    }
    for (auto& sm : hdr.structMacros) {
        if (std::regex_search(sm.name, re))
            results.push_back({sm.name, "macro", sm.structType});
    }
    for (auto& g : hdr.globals) {
        if (std::regex_search(g.name, re))
            results.push_back({g.name, "global", g.cType});
    }

    return results;
}

// ── Fuzzy match (did-you-mean) ──────────────────────────────────────────────

std::vector<HelpCFormatter::SearchResult>
HelpCSearch::fuzzyMatch(const HelpCHeaderInfo& hdr, const std::string& name,
                        size_t maxResults) {
    struct Candidate {
        std::string name;
        std::string kind;
        std::string summary;
        int distance;
    };
    std::vector<Candidate> candidates;

    auto addCandidate = [&](const std::string& n, const std::string& k,
                            const std::string& s) {
        int dist = levenshteinDistance(name, n);
        // Allow reasonable distance (< half the name length + 3)
        int threshold = static_cast<int>(name.size()) / 2 + 3;
        if (dist <= threshold)
            candidates.push_back({n, k, s, dist});
    };

    for (auto& fn : hdr.functions)
        addCandidate(fn.name, "function", functionSummary(fn));
    for (auto& s : hdr.structs)
        addCandidate(s.name, s.isUnion ? "union" : "struct",
                    std::to_string(s.fields.size()) + " fields");
    for (auto& e : hdr.enums)
        addCandidate(e.name, "enum", std::to_string(e.values.size()) + " values");
    for (auto& td : hdr.typedefs)
        addCandidate(td.name, "typedef", "→ " + td.underlyingCType);
    for (auto& m : hdr.macros)
        addCandidate(m.name, "macro", std::to_string(m.value));
    for (auto& sm : hdr.structMacros)
        addCandidate(sm.name, "macro", sm.structType);

    // Sort by distance
    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b) {
            return a.distance < b.distance;
        });

    std::vector<HelpCFormatter::SearchResult> results;
    for (size_t i = 0; i < std::min(candidates.size(), maxResults); i++) {
        results.push_back({candidates[i].name, candidates[i].kind,
                          candidates[i].summary});
    }
    return results;
}
