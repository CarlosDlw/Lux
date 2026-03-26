#pragma once

#include <memory>
#include <string>
#include <antlr4-runtime.h>
#include "generated/LuxLexer.h"
#include "generated/LuxParser.h"

// Holds the full ANTLR4 pipeline; all objects must outlive the parse tree.
struct ParseResult {
    std::unique_ptr<antlr4::ANTLRInputStream>  input;
    std::unique_ptr<LuxLexer>               lexer;
    std::unique_ptr<antlr4::CommonTokenStream> tokens;
    std::unique_ptr<LuxParser>              parser;
    LuxParser::ProgramContext*              tree      = nullptr;
    bool                                       hasErrors = false;
};

class Parser {
public:
    static ParseResult parse(const std::string& filePath);
};
