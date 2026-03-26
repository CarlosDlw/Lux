#pragma once

#include <antlr4-runtime.h>
#include <iostream>
#include <string>

// Silences the default ANTLR error output and reformats syntax errors
// to match the tollvm diagnostic style:  tollvm: <line>:<col>: <message>
class DiagnosticErrorListener : public antlr4::BaseErrorListener {
public:
    void syntaxError(antlr4::Recognizer* /*recognizer*/,
                     antlr4::Token* offendingSymbol,
                     size_t line, size_t charPositionInLine,
                     const std::string& msg,
                     std::exception_ptr /*e*/) override {

        std::string clean = simplify(msg, offendingSymbol);
        std::cerr << "tollvm: " << line << ":" << charPositionInLine
                  << ": " << clean << "\n";
    }

private:
    static std::string simplify(const std::string& msg,
                                antlr4::Token* offending) {
        // "mismatched input 'X' expecting {long set...}" → concise message
        if (msg.rfind("mismatched input", 0) == 0) {
            std::string tok = offending ? quote(offending) : "?";
            return "unexpected " + tok;
        }

        // "extraneous input 'X' expecting ..."
        if (msg.rfind("extraneous input", 0) == 0) {
            std::string tok = offending ? quote(offending) : "?";
            return "unexpected " + tok;
        }

        // "missing 'X' at 'Y'"
        if (msg.rfind("missing", 0) == 0)
            return msg;  // already short enough

        // "no viable alternative at input 'X'"
        if (msg.rfind("no viable alternative", 0) == 0) {
            std::string tok = offending ? quote(offending) : "?";
            return "unexpected " + tok;
        }

        // "token recognition error at: 'X'" (lexer)
        if (msg.rfind("token recognition error", 0) == 0)
            return msg;

        return msg;
    }

    static std::string quote(antlr4::Token* tok) {
        if (tok->getType() == antlr4::Token::EOF)
            return "end of file";
        return "'" + tok->getText() + "'";
    }
};
