#pragma once

#include <string>
#include <vector>
#include "lsp/Diagnostic.h"

// Runs the full parse + semantic check pipeline on a source string
// and produces structured diagnostics suitable for LSP.
class DiagnosticEngine {
public:
    std::vector<Diagnostic> run(const std::string& source);
};
