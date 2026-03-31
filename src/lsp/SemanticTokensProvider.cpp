#include "lsp/SemanticTokensProvider.h"
#include "parser/Parser.h"
#include "generated/LuxLexer.h"
#include "generated/LuxParser.h"

#include <algorithm>
#include <regex>
#include <unordered_set>

// ═══════════════════════════════════════════════════════════════════════
//  Legend
// ═══════════════════════════════════════════════════════════════════════

const std::vector<std::string>& SemanticTokensProvider::tokenTypes() {
    static const std::vector<std::string> types = {
        "namespace", "type", "struct", "enum", "enumMember",
        "function", "method", "parameter", "variable", "property",
        "keyword", "comment", "string", "number", "operator", "macro"
    };
    return types;
}

const std::vector<std::string>& SemanticTokensProvider::tokenModifiers() {
    static const std::vector<std::string> mods = {
        "declaration", "definition", "readonly", "static", "defaultLibrary"
    };
    return mods;
}

// ═══════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════

static inline void emit(std::vector<RawSemanticToken>& out,
                         uint32_t line, uint32_t col, uint32_t len,
                         SemanticTokenType type,
                         uint32_t modifiers = 0) {
    if (len == 0) return;
    out.push_back({line, col, len,
                   static_cast<uint32_t>(type), modifiers});
}

// Keyword token types (by lexer token id)
static bool isKeyword(size_t tokenType) {
    switch (tokenType) {
        case LuxLexer::NAMESPACE: case LuxLexer::USE: case LuxLexer::RET:
        case LuxLexer::STRUCT: case LuxLexer::UNION: case LuxLexer::ENUM:
        case LuxLexer::FN: case LuxLexer::TYPE: case LuxLexer::AS:
        case LuxLexer::IS: case LuxLexer::SIZEOF: case LuxLexer::TYPEOF:
        case LuxLexer::IF: case LuxLexer::ELSE: case LuxLexer::FOR:
        case LuxLexer::IN: case LuxLexer::LOOP: case LuxLexer::WHILE:
        case LuxLexer::DO: case LuxLexer::BREAK: case LuxLexer::CONTINUE:
        case LuxLexer::SWITCH: case LuxLexer::CASE: case LuxLexer::DEFAULT:
        case LuxLexer::SPAWN: case LuxLexer::AWAIT: case LuxLexer::LOCK:
        case LuxLexer::EXTEND: case LuxLexer::TRY: case LuxLexer::CATCH:
        case LuxLexer::FINALLY: case LuxLexer::THROW: case LuxLexer::DEFER:
        case LuxLexer::EXTERN: case LuxLexer::AUTO: case LuxLexer::NULL_LIT:
            return true;
        default:
            return false;
    }
}

static bool isPrimitiveType(size_t tokenType) {
    if (tokenType >= LuxLexer::INT1 && tokenType <= LuxLexer::CSTRING)
        return true;
    // Native collection type keywords are also types
    if (tokenType == LuxLexer::VEC || tokenType == LuxLexer::MAP ||
        tokenType == LuxLexer::SET)
        return true;
    return false;
}

static bool isOperator(size_t tokenType) {
    switch (tokenType) {
        case LuxLexer::PLUS: case LuxLexer::MINUS: case LuxLexer::STAR:
        case LuxLexer::SLASH: case LuxLexer::PERCENT:
        case LuxLexer::EQ: case LuxLexer::NEQ: case LuxLexer::LT:
        case LuxLexer::GT: case LuxLexer::LTE: case LuxLexer::GTE:
        case LuxLexer::LAND: case LuxLexer::LOR: case LuxLexer::NOT:
        case LuxLexer::AMPERSAND: case LuxLexer::PIPE: case LuxLexer::CARET:
        case LuxLexer::TILDE: case LuxLexer::LSHIFT:
        case LuxLexer::INCR: case LuxLexer::DECR:
        case LuxLexer::ASSIGN: case LuxLexer::ARROW:
        case LuxLexer::PLUS_ASSIGN: case LuxLexer::MINUS_ASSIGN:
        case LuxLexer::STAR_ASSIGN: case LuxLexer::SLASH_ASSIGN:
        case LuxLexer::PERCENT_ASSIGN: case LuxLexer::AMP_ASSIGN:
        case LuxLexer::PIPE_ASSIGN: case LuxLexer::CARET_ASSIGN:
        case LuxLexer::LSHIFT_ASSIGN: case LuxLexer::RSHIFT_ASSIGN:
        case LuxLexer::NULLCOAL: case LuxLexer::SPREAD:
        case LuxLexer::RANGE: case LuxLexer::RANGE_INCL:
        case LuxLexer::QUESTION:
            return true;
        default:
            return false;
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Collect contextual identifiers from the parse tree
// ═══════════════════════════════════════════════════════════════════════

struct IdentClassification {
    SemanticTokenType type;
    uint32_t modifiers;
};

// Key: "line:col" → classification
using IdentMap = std::unordered_map<std::string, IdentClassification>;

static std::string key(antlr4::Token* tok) {
    return std::to_string(tok->getLine() - 1) + ":" +
           std::to_string(tok->getCharPositionInLine());
}

static std::string key(antlr4::tree::TerminalNode* node) {
    return key(node->getSymbol());
}

static void classifyIdent(IdentMap& map, antlr4::tree::TerminalNode* node,
                           SemanticTokenType type, uint32_t modifiers = 0) {
    if (!node) return;
    map[key(node)] = {type, modifiers};
}

// Walk the parse tree to classify IDENTIFIER tokens by their context.
static void walkTree(IdentMap& map, antlr4::tree::ParseTree* node) {
    if (!node) return;

    // ── namespace ──
    if (auto* ctx = dynamic_cast<LuxParser::NamespaceDeclContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(),
                      SemanticTokenType::Namespace,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── use ── modulePath identifiers are namespaces
    else if (auto* ctx = dynamic_cast<LuxParser::ModulePathContext*>(node)) {
        for (auto* id : ctx->IDENTIFIER())
            classifyIdent(map, id, SemanticTokenType::Namespace);
    }
    else if (auto* ctx = dynamic_cast<LuxParser::UseItemContext*>(node)) {
        // The last IDENTIFIER is the imported symbol (function/type)
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Function);
    }
    else if (auto* ctx = dynamic_cast<LuxParser::UseGroupContext*>(node)) {
        for (auto* id : ctx->IDENTIFIER())
            classifyIdent(map, id, SemanticTokenType::Function);
    }

    // ── struct ──
    else if (auto* ctx = dynamic_cast<LuxParser::StructDeclContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Struct,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration) |
                      static_cast<uint32_t>(SemanticTokenMod::Definition));
    }
    else if (auto* ctx = dynamic_cast<LuxParser::StructFieldContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Property,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── union ──
    else if (auto* ctx = dynamic_cast<LuxParser::UnionDeclContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Struct,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration) |
                      static_cast<uint32_t>(SemanticTokenMod::Definition));
    }
    else if (auto* ctx = dynamic_cast<LuxParser::UnionFieldContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Property,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── enum ──
    else if (auto* ctx = dynamic_cast<LuxParser::EnumDeclContext*>(node)) {
        auto ids = ctx->IDENTIFIER();
        if (!ids.empty()) {
            classifyIdent(map, ids[0], SemanticTokenType::Enum,
                          static_cast<uint32_t>(SemanticTokenMod::Declaration) |
                          static_cast<uint32_t>(SemanticTokenMod::Definition));
            for (size_t i = 1; i < ids.size(); ++i)
                classifyIdent(map, ids[i], SemanticTokenType::EnumMember,
                              static_cast<uint32_t>(SemanticTokenMod::Declaration));
        }
    }

    // ── type alias ──
    else if (auto* ctx = dynamic_cast<LuxParser::TypeAliasDeclContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Type,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── function decl ──
    else if (auto* ctx = dynamic_cast<LuxParser::FunctionDeclContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Function,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration) |
                      static_cast<uint32_t>(SemanticTokenMod::Definition));
    }

    // ── extern decl ──
    else if (auto* ctx = dynamic_cast<LuxParser::ExternDeclContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Function,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── extend block ──
    else if (auto* ctx = dynamic_cast<LuxParser::ExtendDeclContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Struct);
    }
    else if (auto* ctx = dynamic_cast<LuxParser::ExtendMethodContext*>(node)) {
        auto ids = ctx->IDENTIFIER();
        if (ids.size() >= 2) {
            // ids[0] is return type (or self param name), ids[1] is method name
            classifyIdent(map, ids[1], SemanticTokenType::Method,
                          static_cast<uint32_t>(SemanticTokenMod::Declaration) |
                          static_cast<uint32_t>(SemanticTokenMod::Definition));
        } else if (ids.size() == 1) {
            classifyIdent(map, ids[0], SemanticTokenType::Method,
                          static_cast<uint32_t>(SemanticTokenMod::Declaration) |
                          static_cast<uint32_t>(SemanticTokenMod::Definition));
        }
    }

    // ── parameters ──
    else if (auto* ctx = dynamic_cast<LuxParser::ParamContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Parameter,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }
    else if (auto* ctx = dynamic_cast<LuxParser::ExternParamContext*>(node)) {
        if (ctx->IDENTIFIER())
            classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Parameter,
                          static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── variable declarations ──
    else if (auto* ctx = dynamic_cast<LuxParser::VarDeclStmtContext*>(node)) {
        for (auto* id : ctx->IDENTIFIER())
            classifyIdent(map, id, SemanticTokenType::Variable,
                          static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── for-in variable ──
    else if (auto* ctx = dynamic_cast<LuxParser::ForInStmtContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Variable,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }
    else if (auto* ctx = dynamic_cast<LuxParser::ForClassicStmtContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Variable,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── catch variable ──
    else if (auto* ctx = dynamic_cast<LuxParser::CatchClauseContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Variable,
                      static_cast<uint32_t>(SemanticTokenMod::Declaration));
    }

    // ── function/method call expressions ──
    else if (auto* ctx = dynamic_cast<LuxParser::FnCallExprContext*>(node)) {
        // If the callee is a bare identifier, classify it as function
        if (auto* identExpr = dynamic_cast<LuxParser::IdentExprContext*>(ctx->expression())) {
            classifyIdent(map, identExpr->IDENTIFIER(), SemanticTokenType::Function);
        }
    }
    else if (auto* ctx = dynamic_cast<LuxParser::MethodCallExprContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Method);
    }
    else if (auto* ctx = dynamic_cast<LuxParser::StaticMethodCallExprContext*>(node)) {
        auto ids = ctx->IDENTIFIER();
        if (ids.size() >= 2) {
            classifyIdent(map, ids[0], SemanticTokenType::Type);
            classifyIdent(map, ids[1], SemanticTokenType::Method,
                          static_cast<uint32_t>(SemanticTokenMod::Static));
        }
    }

    // ── call statement ──
    else if (auto* ctx = dynamic_cast<LuxParser::CallStmtContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Function);
    }

    // ── field access ──
    else if (auto* ctx = dynamic_cast<LuxParser::FieldAccessExprContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Property);
    }
    else if (auto* ctx = dynamic_cast<LuxParser::ArrowAccessExprContext*>(node)) {
        classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Property);
    }

    // ── enum access ──
    else if (auto* ctx = dynamic_cast<LuxParser::EnumAccessExprContext*>(node)) {
        auto ids = ctx->IDENTIFIER();
        if (ids.size() >= 2) {
            classifyIdent(map, ids[0], SemanticTokenType::Enum);
            classifyIdent(map, ids[1], SemanticTokenType::EnumMember);
        }
    }

    // ── struct literal ──
    else if (auto* ctx = dynamic_cast<LuxParser::StructLitExprContext*>(node)) {
        auto ids = ctx->IDENTIFIER();
        if (!ids.empty()) {
            classifyIdent(map, ids[0], SemanticTokenType::Struct);
            // Rest are field names
            for (size_t i = 1; i < ids.size(); ++i)
                classifyIdent(map, ids[i], SemanticTokenType::Property);
        }
    }

    // ── typeSpec IDENTIFIER → user-defined type ──
    else if (auto* ctx = dynamic_cast<LuxParser::TypeSpecContext*>(node)) {
        // Only classify if this is a plain IDENTIFIER typeSpec (user type)
        // and there's no other child rule (not pointer, array, fn, etc.)
        if (ctx->IDENTIFIER() && !ctx->LT() &&
            ctx->children.size() == 1) {
            classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Type);
        }
        // Generic type: Vec<int32> — the IDENTIFIER before LT
        else if (ctx->IDENTIFIER() && ctx->LT()) {
            classifyIdent(map, ctx->IDENTIFIER(), SemanticTokenType::Type);
        }
    }

    // Recurse into children
    for (size_t i = 0; i < node->children.size(); ++i) {
        walkTree(map, node->children[i]);
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Collect comments (skipped by ANTLR lexer, so we scan manually)
// ═══════════════════════════════════════════════════════════════════════

static void collectComments(std::vector<RawSemanticToken>& out,
                             const std::string& source) {
    uint32_t line = 0;
    uint32_t col  = 0;
    size_t i = 0;

    while (i < source.size()) {
        if (source[i] == '\n') {
            ++line; col = 0; ++i;
            continue;
        }
        if (source[i] == '\r') {
            ++line; col = 0; ++i;
            if (i < source.size() && source[i] == '\n') ++i;
            continue;
        }

        // Line comment: // ...
        if (i + 1 < source.size() && source[i] == '/' && source[i + 1] == '/') {
            uint32_t startCol = col;
            size_t start = i;
            while (i < source.size() && source[i] != '\n' && source[i] != '\r')
                ++i;
            uint32_t len = static_cast<uint32_t>(i - start);
            emit(out, line, startCol, len, SemanticTokenType::Comment);
            col += len;
            continue;
        }

        // Block comment: /* ... */
        if (i + 1 < source.size() && source[i] == '/' && source[i + 1] == '*') {
            uint32_t startLine = line;
            uint32_t startCol  = col;
            size_t start = i;
            i += 2; col += 2;

            while (i < source.size()) {
                if (source[i] == '\n') {
                    // Emit the current line portion of the comment
                    uint32_t len = static_cast<uint32_t>(i - start);
                    emit(out, startLine, startCol, len, SemanticTokenType::Comment);
                    ++line; col = 0; ++i;
                    start = i;
                    startLine = line;
                    startCol = 0;
                } else if (source[i] == '\r') {
                    uint32_t len = static_cast<uint32_t>(i - start);
                    emit(out, startLine, startCol, len, SemanticTokenType::Comment);
                    ++line; col = 0; ++i;
                    if (i < source.size() && source[i] == '\n') ++i;
                    start = i;
                    startLine = line;
                    startCol = 0;
                } else if (i + 1 < source.size() &&
                           source[i] == '*' && source[i + 1] == '/') {
                    i += 2; col += 2;
                    uint32_t len = static_cast<uint32_t>(i - start);
                    emit(out, startLine, startCol, len, SemanticTokenType::Comment);
                    break;
                } else {
                    ++col; ++i;
                }
            }
            continue;
        }

        // Skip string and char literals (so we don't falsely detect // inside them)
        if (source[i] == '"') {
            ++i; ++col;
            while (i < source.size() && source[i] != '"' && source[i] != '\n') {
                if (source[i] == '\\') { ++i; ++col; }
                ++i; ++col;
            }
            if (i < source.size() && source[i] == '"') { ++i; ++col; }
            continue;
        }
        if (source[i] == '\'') {
            ++i; ++col;
            while (i < source.size() && source[i] != '\'' && source[i] != '\n') {
                if (source[i] == '\\') { ++i; ++col; }
                ++i; ++col;
            }
            if (i < source.size() && source[i] == '\'') { ++i; ++col; }
            continue;
        }

        ++col; ++i;
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Main tokenize
// ═══════════════════════════════════════════════════════════════════════

std::vector<uint32_t> SemanticTokensProvider::tokenize(const std::string& source) {
    if (source.empty()) return {};

    std::vector<RawSemanticToken> raw;

    // 1. Parse with ANTLR
    auto pr = Parser::parseString(source);
    if (!pr.tree) return {};

    // 2. Walk parse tree to classify identifiers by context
    IdentMap identMap;
    walkTree(identMap, pr.tree);

    // 3. Walk lexer tokens and emit semantic tokens
    pr.tokens->fill();
    for (auto* tok : pr.tokens->getTokens()) {
        if (tok->getType() == antlr4::Token::EOF) break;

        uint32_t line = static_cast<uint32_t>(tok->getLine()) - 1; // ANTLR is 1-based
        uint32_t col  = static_cast<uint32_t>(tok->getCharPositionInLine());
        uint32_t len  = static_cast<uint32_t>(tok->getText().size());
        size_t type   = tok->getType();

        // Keywords
        if (isKeyword(type)) {
            emit(raw, line, col, len, SemanticTokenType::Keyword);
        }
        // Primitive types
        else if (isPrimitiveType(type)) {
            emit(raw, line, col, len, SemanticTokenType::Type,
                 static_cast<uint32_t>(SemanticTokenMod::DefaultLib));
        }
        // Literals
        else if (type == LuxLexer::INT_LIT || type == LuxLexer::FLOAT_LIT ||
                 type == LuxLexer::HEX_LIT || type == LuxLexer::OCT_LIT ||
                 type == LuxLexer::BIN_LIT) {
            emit(raw, line, col, len, SemanticTokenType::Number);
        }
        else if (type == LuxLexer::BOOL_LIT) {
            emit(raw, line, col, len, SemanticTokenType::Keyword);
        }
        else if (type == LuxLexer::STR_LIT || type == LuxLexer::C_STR_LIT ||
                 type == LuxLexer::CHAR_LIT) {
            emit(raw, line, col, len, SemanticTokenType::String);
        }
        // Operators
        else if (isOperator(type)) {
            emit(raw, line, col, len, SemanticTokenType::Operator);
        }
        // Include directives
        else if (type == LuxLexer::INCLUDE_SYS || type == LuxLexer::INCLUDE_LOCAL) {
            emit(raw, line, col, len, SemanticTokenType::Macro);
        }
        // Identifiers — look up in the context map
        else if (type == LuxLexer::IDENTIFIER) {
            std::string k = std::to_string(line) + ":" + std::to_string(col);
            auto it = identMap.find(k);
            if (it != identMap.end()) {
                emit(raw, line, col, len, it->second.type, it->second.modifiers);
            } else {
                // Unclassified identifier → default to variable
                emit(raw, line, col, len, SemanticTokenType::Variable);
            }
        }
    }

    // 4. Collect comments (skipped by ANTLR)
    collectComments(raw, source);

    // 5. Sort by position
    std::sort(raw.begin(), raw.end(), [](const RawSemanticToken& a,
                                          const RawSemanticToken& b) {
        return a.line < b.line || (a.line == b.line && a.col < b.col);
    });

    // 6. Delta-encode
    std::vector<uint32_t> encoded;
    encoded.reserve(raw.size() * 5);
    uint32_t prevLine = 0;
    uint32_t prevCol  = 0;

    for (auto& tok : raw) {
        uint32_t deltaLine = tok.line - prevLine;
        uint32_t deltaCol  = (deltaLine == 0) ? tok.col - prevCol : tok.col;
        encoded.push_back(deltaLine);
        encoded.push_back(deltaCol);
        encoded.push_back(tok.length);
        encoded.push_back(tok.type);
        encoded.push_back(tok.modifiers);
        prevLine = tok.line;
        prevCol  = tok.col;
    }

    return encoded;
}
