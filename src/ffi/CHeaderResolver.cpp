#include "ffi/CHeaderResolver.h"

#include <clang-c/Index.h>

#include <iostream>
#include <algorithm>
#include <array>
#include <cstdio>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>

// Discover system include paths by querying clang.
static std::vector<std::string> discoverSystemIncludes() {
    std::vector<std::string> paths;

    std::array<char, 4096> buffer;
    std::string output;

    FILE* pipe = popen("clang -E -x c -v /dev/null 2>&1", "r");
    if (!pipe) return paths;

    while (fgets(buffer.data(), buffer.size(), pipe))
        output += buffer.data();
    pclose(pipe);

    // Parse lines between "#include <...> search starts here:" and "End of search list."
    auto startMarker = output.find("#include <...> search starts here:");
    auto endMarker   = output.find("End of search list.");
    if (startMarker == std::string::npos || endMarker == std::string::npos)
        return paths;

    std::istringstream stream(output.substr(
        startMarker + 34, endMarker - startMarker - 34));
    std::string line;
    while (std::getline(stream, line)) {
        // Trim whitespace
        auto first = line.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        auto trimmed = line.substr(first);
        if (!trimmed.empty())
            paths.push_back("-isystem" + trimmed);
    }

    return paths;
}

// ── Detect required linker libraries for a given header ──────────────────

// Well-known header → linker flag mapping.
// Each entry maps a header name (or prefix) to the -l flag it needs.
static const std::unordered_map<std::string, std::string>& knownHeaderLibs() {
    static const std::unordered_map<std::string, std::string> table = {
        // Graphics / Windowing
        {"raylib.h",            "-lraylib"},
        {"SDL.h",               "-lSDL2"},
        {"SDL2/SDL.h",          "-lSDL2"},
        {"SDL_image.h",         "-lSDL2_image"},
        {"SDL_ttf.h",           "-lSDL2_ttf"},
        {"SDL_mixer.h",         "-lSDL2_mixer"},
        {"SDL_net.h",           "-lSDL2_net"},
        {"GLFW/glfw3.h",        "-lglfw"},
        {"GL/gl.h",             "-lGL"},
        {"GL/glu.h",            "-lGLU"},
        {"GL/glew.h",           "-lGLEW"},
        {"vulkan/vulkan.h",     "-lvulkan"},
        // Networking / Web
        {"curl/curl.h",         "-lcurl"},
        {"openssl/ssl.h",       "-lssl"},
        {"openssl/crypto.h",    "-lcrypto"},
        {"openssl/evp.h",       "-lcrypto"},
        // Compression
        {"zlib.h",              "-lz"},
        {"bzlib.h",             "-lbz2"},
        {"lzma.h",              "-llzma"},
        {"zstd.h",              "-lzstd"},
        // Audio / Media
        {"portaudio.h",         "-lportaudio"},
        {"sndfile.h",           "-lsndfile"},
        {"mpg123.h",            "-lmpg123"},
        // Image
        {"png.h",               "-lpng"},
        {"jpeglib.h",           "-ljpeg"},
        {"tiff.h",              "-ltiff"},
        // Database
        {"sqlite3.h",           "-lsqlite3"},
        {"libpq-fe.h",          "-lpq"},
        {"mysql/mysql.h",       "-lmysqlclient"},
        // System
        {"readline/readline.h", "-lreadline"},
        {"ncurses.h",           "-lncurses"},
        {"curses.h",            "-lcurses"},
        // XML / JSON / Parsing
        {"libxml/parser.h",     "-lxml2"},
        {"expat.h",             "-lexpat"},
        {"yaml.h",              "-lyaml"},
    };
    return table;
}

// Try pkg-config to discover linker flags for a header.
// Checks if a .pc file exists whose name matches the header base name.
static std::string tryPkgConfig(const std::string& headerName) {
    // Derive candidate pkg names from the header.
    // e.g. "raylib.h" → "raylib", "SDL2/SDL.h" → "sdl2"
    namespace fs = std::filesystem;
    std::string base = fs::path(headerName).stem().string();

    // Lowercase for pkg-config lookup
    std::string lower = base;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // Try a few candidate names
    std::vector<std::string> candidates = {lower, base};

    // For paths like "SDL2/SDL.h", also try the directory component
    auto parent = fs::path(headerName).parent_path().string();
    if (!parent.empty()) {
        std::string parentLower = parent;
        std::transform(parentLower.begin(), parentLower.end(),
                       parentLower.begin(), ::tolower);
        candidates.insert(candidates.begin(), parentLower);
    }

    for (auto& name : candidates) {
        std::string cmd = "pkg-config --libs " + name + " 2>/dev/null";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) continue;

        std::array<char, 512> buffer;
        std::string output;
        while (fgets(buffer.data(), buffer.size(), pipe))
            output += buffer.data();
        int rc = pclose(pipe);

        if (rc == 0 && !output.empty()) {
            // Trim trailing whitespace
            while (!output.empty() &&
                   (output.back() == '\n' || output.back() == ' '))
                output.pop_back();
            return output;
        }
    }
    return {};
}

CHeaderResolver::CHeaderResolver(TypeRegistry& registry, CBindings& bindings,
                                 const std::vector<std::string>& includePaths)
    : mapper_(registry, bindings), bindings_(bindings),
      includePaths_(includePaths), systemIncludes_(discoverSystemIncludes()) {}

std::string CHeaderResolver::extractSystemHeader(const std::string& tokenText) {
    // "#include <stdio.h>" → "stdio.h"
    auto start = tokenText.find('<');
    auto end   = tokenText.find('>');
    if (start == std::string::npos || end == std::string::npos || end <= start + 1)
        return {};
    return tokenText.substr(start + 1, end - start - 1);
}

std::string CHeaderResolver::extractLocalHeader(const std::string& tokenText) {
    // '#include "mylib.h"' → "mylib.h"
    auto first  = tokenText.find('"');
    auto second = tokenText.find('"', first + 1);
    if (first == std::string::npos || second == std::string::npos || second <= first + 1)
        return {};
    return tokenText.substr(first + 1, second - first - 1);
}

bool CHeaderResolver::resolveSystemHeader(const std::string& headerName) {
    if (parsedHeaders_.count(headerName)) return true;

    std::string source = "#include <" + headerName + ">\n";

    std::vector<std::string> clangArgs;
    for (auto& path : includePaths_) {
        if (path.size() > 2 && path[0] == '-' && path[1] == 'I') {
            clangArgs.push_back("-I" + path.substr(2));
        } else {
            clangArgs.push_back("-I" + path);
        }
    }
    for (auto& si : systemIncludes_)
        clangArgs.push_back(si);

    if (!parseHeader(source, clangArgs)) return false;
    parsedHeaders_.insert(headerName);

    // Detect required linker library for this header
    detectRequiredLib(headerName);

    return true;
}

bool CHeaderResolver::resolveLocalHeader(const std::string& headerName,
                                          const std::string& basePath) {
    std::string key = basePath + "/" + headerName;
    if (parsedHeaders_.count(key)) return true;

    std::string source = "#include \"" + headerName + "\"\n";

    std::vector<std::string> clangArgs;
    clangArgs.push_back("-I" + basePath);
    for (auto& path : includePaths_) {
        if (path.size() > 2 && path[0] == '-' && path[1] == 'I') {
            clangArgs.push_back("-I" + path.substr(2));
        } else {
            clangArgs.push_back("-I" + path);
        }
    }
    for (auto& si : systemIncludes_)
        clangArgs.push_back(si);

    if (!parseHeader(source, clangArgs)) return false;
    parsedHeaders_.insert(key);
    return true;
}

// libclang visitor callback
struct VisitorData {
    CTypeMapper* mapper;
    CBindings*   bindings;
};

// ── Macro constant evaluation helpers ────────────────────────────────────

static bool isTypeKeyword(const std::string& tok) {
    static const std::unordered_set<std::string> kw = {
        "void", "char", "short", "int", "long", "float", "double",
        "signed", "unsigned", "const", "volatile", "restrict",
        "size_t", "ssize_t", "ptrdiff_t", "intptr_t", "uintptr_t",
        "int8_t", "int16_t", "int32_t", "int64_t",
        "uint8_t", "uint16_t", "uint32_t", "uint64_t"
    };
    return kw.count(tok) > 0;
}

static std::string stripIntSuffix(const std::string& tok) {
    std::string s = tok;
    while (!s.empty()) {
        char c = s.back();
        if (c == 'u' || c == 'U' || c == 'l' || c == 'L')
            s.pop_back();
        else break;
    }
    return s;
}

static bool parseIntLiteral(const std::string& raw, int64_t& val) {
    std::string tok = stripIntSuffix(raw);
    if (tok.empty()) return false;
    try {
        size_t pos = 0;
        if (tok.size() > 2 && tok[0] == '0' && (tok[1] == 'x' || tok[1] == 'X'))
            val = std::stoll(tok, &pos, 16);
        else if (tok.size() > 1 && tok[0] == '0')
            val = std::stoll(tok, &pos, 8);
        else
            val = std::stoll(tok, &pos, 10);
        return pos == tok.size();
    } catch (...) {
        return false;
    }
}

static bool parseCharLiteral(const std::string& tok, int64_t& val) {
    if (tok.size() < 3 || tok.front() != '\'' || tok.back() != '\'')
        return false;
    std::string inner = tok.substr(1, tok.size() - 2);
    if (inner.empty()) return false;
    if (inner[0] != '\\') {
        val = static_cast<unsigned char>(inner[0]);
        return inner.size() == 1;
    }
    // Escape sequences
    if (inner.size() < 2) return false;
    switch (inner[1]) {
        case 'n':  val = '\n'; return true;
        case 't':  val = '\t'; return true;
        case 'r':  val = '\r'; return true;
        case '0':  val = '\0'; return true;
        case '\\': val = '\\'; return true;
        case '\'': val = '\''; return true;
        case '"':  val = '"';  return true;
        case 'a':  val = '\a'; return true;
        case 'b':  val = '\b'; return true;
        case 'f':  val = '\f'; return true;
        case 'v':  val = '\v'; return true;
        default:
            // Octal: \0nn
            if (inner[1] >= '0' && inner[1] <= '7') {
                val = std::stoll(inner.substr(1), nullptr, 8);
                return true;
            }
            // Hex: \xNN
            if (inner[1] == 'x' && inner.size() > 2) {
                val = std::stoll(inner.substr(2), nullptr, 16);
                return true;
            }
            return false;
    }
}

// Forward declaration for recursive descent parser
static bool evalExpr(const std::vector<std::string>& toks, size_t& pos,
                     int64_t& val, bool& hasVoidCast);

static bool evalPrimary(const std::vector<std::string>& toks, size_t& pos,
                        int64_t& val, bool& hasVoidCast) {
    if (pos >= toks.size()) return false;

    // Unary minus
    if (toks[pos] == "-") {
        pos++;
        if (!evalPrimary(toks, pos, val, hasVoidCast)) return false;
        val = -val;
        return true;
    }
    // Bitwise NOT
    if (toks[pos] == "~") {
        pos++;
        if (!evalPrimary(toks, pos, val, hasVoidCast)) return false;
        val = ~val;
        return true;
    }

    // Parenthesized expression or cast
    if (toks[pos] == "(") {
        size_t savedPos = pos;
        pos++; // skip (

        // Check if this is a C type cast: (type...) expr
        if (pos < toks.size() &&
            (isTypeKeyword(toks[pos]) || toks[pos] == "*")) {
            // It's a cast — track if it includes void*
            int depth = 1;
            while (pos < toks.size() && depth > 0) {
                if (toks[pos] == "void" && pos + 1 < toks.size() &&
                    toks[pos + 1] == "*")
                    hasVoidCast = true;
                if (toks[pos] == "(") depth++;
                else if (toks[pos] == ")") depth--;
                pos++;
            }
            return evalPrimary(toks, pos, val, hasVoidCast);
        }

        // Regular parenthesized expression
        if (!evalExpr(toks, pos, val, hasVoidCast)) return false;
        if (pos >= toks.size() || toks[pos] != ")") return false;
        pos++; // skip )
        return true;
    }

    // Character literal
    if (!toks[pos].empty() && toks[pos][0] == '\'')
        return parseCharLiteral(toks[pos++], val);

    // Integer literal
    return parseIntLiteral(toks[pos++], val);
}

static bool evalShift(const std::vector<std::string>& toks, size_t& pos,
                      int64_t& val, bool& hasVoidCast) {
    if (!evalPrimary(toks, pos, val, hasVoidCast)) return false;
    while (pos < toks.size()) {
        if (toks[pos] == "<<") {
            pos++;
            int64_t rhs;
            if (!evalPrimary(toks, pos, rhs, hasVoidCast)) return false;
            val <<= rhs;
        } else if (toks[pos] == ">>") {
            pos++;
            int64_t rhs;
            if (!evalPrimary(toks, pos, rhs, hasVoidCast)) return false;
            val >>= rhs;
        } else break;
    }
    return true;
}

static bool evalBitAnd(const std::vector<std::string>& toks, size_t& pos,
                       int64_t& val, bool& hasVoidCast) {
    if (!evalShift(toks, pos, val, hasVoidCast)) return false;
    while (pos < toks.size() && toks[pos] == "&") {
        pos++;
        int64_t rhs;
        if (!evalShift(toks, pos, rhs, hasVoidCast)) return false;
        val &= rhs;
    }
    return true;
}

static bool evalBitOr(const std::vector<std::string>& toks, size_t& pos,
                      int64_t& val, bool& hasVoidCast) {
    if (!evalBitAnd(toks, pos, val, hasVoidCast)) return false;
    while (pos < toks.size() && toks[pos] == "|") {
        pos++;
        int64_t rhs;
        if (!evalBitAnd(toks, pos, rhs, hasVoidCast)) return false;
        val |= rhs;
    }
    return true;
}

static bool evalExpr(const std::vector<std::string>& toks, size_t& pos,
                     int64_t& val, bool& hasVoidCast) {
    return evalBitOr(toks, pos, val, hasVoidCast);
}

// Try to evaluate macro body tokens as a simple integer constant.
// Returns true if successful, sets `result` to the value.
// Sets `isNull` to true if the macro looks like a NULL pointer constant.
static bool tryEvalMacroTokens(const std::vector<std::string>& tokens,
                               int64_t& result, bool& isNull) {
    if (tokens.empty()) return false;
    isNull = false;
    bool hasVoidCast = false;
    size_t pos = 0;
    if (!evalExpr(tokens, pos, result, hasVoidCast)) return false;
    if (pos != tokens.size()) return false; // must consume all tokens
    if (hasVoidCast && result == 0) isNull = true;
    return true;
}

// Try to parse a macro body as a struct compound literal.
// Patterns recognized:
//   CLITERAL(Color){245, 245, 245, 255}     (raylib style)
//   (Color){245, 245, 245, 255}              (C99 compound literal)
static bool tryEvalStructLiteralMacro(const std::vector<std::string>& tokens,
                                      std::string& outType,
                                      std::vector<int64_t>& outValues) {
    if (tokens.size() < 5) return false;

    size_t pos = 0;

    // Pattern 1: CLITERAL ( Type ) { ... }
    // Pattern 2: ( Type ) { ... }
    if (tokens[pos] == "CLITERAL" || tokens[pos] == "CLIT") {
        pos++; // skip CLITERAL
    }

    if (pos >= tokens.size() || tokens[pos] != "(") return false;
    pos++; // skip (

    if (pos >= tokens.size()) return false;
    outType = tokens[pos];
    pos++; // skip Type

    // Handle pointer types like (Type *)
    while (pos < tokens.size() && tokens[pos] == "*") pos++;

    if (pos >= tokens.size() || tokens[pos] != ")") return false;
    pos++; // skip )

    if (pos >= tokens.size() || tokens[pos] != "{") return false;
    pos++; // skip {

    // Parse comma-separated integer values
    outValues.clear();
    while (pos < tokens.size() && tokens[pos] != "}") {
        if (tokens[pos] == ",") { pos++; continue; }

        // Try parsing as integer (supports negative values)
        int64_t val = 0;
        bool negative = false;
        if (tokens[pos] == "-") {
            negative = true;
            pos++;
            if (pos >= tokens.size() || tokens[pos] == "}" || tokens[pos] == ",")
                return false;
        }
        if (!parseIntLiteral(tokens[pos], val)) return false;
        if (negative) val = -val;
        outValues.push_back(val);
        pos++;
    }

    if (pos >= tokens.size() || tokens[pos] != "}") return false;
    pos++; // skip }

    // Must have consumed all tokens
    if (pos != tokens.size()) return false;

    return !outType.empty() && !outValues.empty();
}

// ── libclang visitor callback ────────────────────────────────────────────

static CXChildVisitResult cursorVisitor(CXCursor cursor, CXCursor /*parent*/,
                                         CXClientData clientData) {
    auto* data = static_cast<VisitorData*>(clientData);

    // Only process top-level declarations from the included headers
    CXSourceLocation loc = clang_getCursorLocation(cursor);
    if (clang_Location_isInSystemHeader(loc) == 0) {
        // For system includes, we want system header declarations
        // For local includes, both are fine
        // libclang marks everything from #include <x.h> as system header
        // so we actually want to process system headers too
    }

    CXCursorKind kind = clang_getCursorKind(cursor);

    if (kind == CXCursor_FunctionDecl) {
        CXString cxName = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxName);
        clang_disposeString(cxName);

        // Skip if already registered
        if (data->bindings->findFunction(name))
            return CXChildVisit_Continue;

        CXType funcType = clang_getCursorType(cursor);
        CXType retType  = clang_getResultType(funcType);

        auto* retTI = data->mapper->map(retType);
        if (!retTI) return CXChildVisit_Continue;

        int numArgs = clang_Cursor_getNumArguments(cursor);
        std::vector<const TypeInfo*> paramTypes;
        bool allMapped = true;

        for (int i = 0; i < numArgs; i++) {
            CXCursor argCursor = clang_Cursor_getArgument(cursor, i);
            CXType argType = clang_getCursorType(argCursor);
            auto* argTI = data->mapper->map(argType);
            if (!argTI) {
                allMapped = false;
                break;
            }
            paramTypes.push_back(argTI);
        }

        if (!allMapped) return CXChildVisit_Continue;

        bool isVariadic = (clang_isFunctionTypeVariadic(funcType) != 0);

        CFunction cfunc;
        cfunc.name       = std::move(name);
        cfunc.returnType = retTI;
        cfunc.paramTypes = std::move(paramTypes);
        cfunc.isVariadic = isVariadic;
        data->bindings->addFunction(std::move(cfunc));
    }

    // ── Struct declarations ──────────────────────────────────────────
    if (kind == CXCursor_StructDecl || kind == CXCursor_UnionDecl) {
        // Only process definitions (skip forward declarations)
        if (clang_isCursorDefinition(cursor) == 0)
            return CXChildVisit_Continue;

        CXString cxName = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxName);
        clang_disposeString(cxName);

        // Skip anonymous structs/unions
        if (name.empty()) return CXChildVisit_Continue;

        // Skip if already registered
        if (data->bindings->findStruct(name))
            return CXChildVisit_Continue;

        // mapStruct handles field extraction + registration
        // (unions are handled via byte-size representation)
        data->mapper->mapStruct(cursor);
    }

    // ── Enum declarations ────────────────────────────────────────────
    if (kind == CXCursor_EnumDecl) {
        if (clang_isCursorDefinition(cursor) == 0)
            return CXChildVisit_Continue;

        CXString cxName = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxName);
        clang_disposeString(cxName);

        if (name.empty()) return CXChildVisit_Continue;
        if (data->bindings->findEnum(name))
            return CXChildVisit_Continue;

        CEnum cenum;
        cenum.name = name;

        // Visit enum constants
        clang_visitChildren(cursor,
            [](CXCursor child, CXCursor /*parent*/, CXClientData data)
                -> CXChildVisitResult {
                auto* cenum = static_cast<CEnum*>(data);
                if (clang_getCursorKind(child) != CXCursor_EnumConstantDecl)
                    return CXChildVisit_Continue;

                CXString cxConstName = clang_getCursorSpelling(child);
                std::string constName = clang_getCString(cxConstName);
                clang_disposeString(cxConstName);

                int64_t value = clang_getEnumConstantDeclValue(child);
                cenum->values.push_back({ constName, value });
                return CXChildVisit_Continue;
            },
            &cenum
        );

        data->bindings->addEnum(std::move(cenum));
    }

    // ── Typedef declarations ─────────────────────────────────────────
    if (kind == CXCursor_TypedefDecl) {
        CXString cxName = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxName);
        clang_disposeString(cxName);

        if (name.empty()) return CXChildVisit_Continue;
        if (data->bindings->findTypedef(name))
            return CXChildVisit_Continue;

        CXType underlyingCX = clang_getTypedefDeclUnderlyingType(cursor);
        auto* underlyingTI = data->mapper->map(underlyingCX);
        if (!underlyingTI) return CXChildVisit_Continue;

        CTypedef ctdef;
        ctdef.name = name;
        ctdef.underlying = underlyingTI;
        data->bindings->addTypedef(std::move(ctdef));
    }

    // ── Macro definitions (#define constants) ────────────────────────
    if (kind == CXCursor_MacroDefinition) {
        // Skip function-like macros: #define FOO(x) ...
        if (clang_Cursor_isMacroFunctionLike(cursor))
            return CXChildVisit_Continue;
        // Skip compiler builtins
        if (clang_Cursor_isMacroBuiltin(cursor))
            return CXChildVisit_Continue;

        CXString cxName = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxName);
        clang_disposeString(cxName);

        if (name.empty()) return CXChildVisit_Continue;

        // Skip internal/reserved macros (start with _)
        if (name[0] == '_') return CXChildVisit_Continue;

        // Skip if name conflicts with an already-registered symbol
        if (data->bindings->findMacro(name))
            return CXChildVisit_Continue;

        // Tokenize the macro body
        CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
        CXSourceRange range = clang_getCursorExtent(cursor);
        CXToken* tokens = nullptr;
        unsigned numTokens = 0;
        clang_tokenize(tu, range, &tokens, &numTokens);

        // First token is the macro name itself; rest is the body
        std::vector<std::string> bodyTokens;
        for (unsigned i = 1; i < numTokens; i++) {
            CXString ts = clang_getTokenSpelling(tu, tokens[i]);
            bodyTokens.push_back(clang_getCString(ts));
            clang_disposeString(ts);
        }
        clang_disposeTokens(tu, tokens, numTokens);

        int64_t value = 0;
        bool isNull = false;
        if (tryEvalMacroTokens(bodyTokens, value, isNull)) {
            CMacro cm;
            cm.name   = name;
            cm.value  = value;
            cm.isNull = isNull;
            data->bindings->addMacro(std::move(cm));
        } else {
            // Try as struct compound literal: CLITERAL(Type){...} or (Type){...}
            std::string structType;
            std::vector<int64_t> fieldValues;
            if (tryEvalStructLiteralMacro(bodyTokens, structType, fieldValues)) {
                CStructMacro sm;
                sm.name        = name;
                sm.structType  = structType;
                sm.fieldValues = std::move(fieldValues);
                data->bindings->addStructMacro(std::move(sm));
            }
        }
    }

    // ── Global variable declarations (extern) ────────────────────────
    if (kind == CXCursor_VarDecl) {
        CXLinkageKind linkage = clang_getCursorLinkage(cursor);
        // Only extern globals (not static locals, etc.)
        if (linkage != CXLinkage_External)
            return CXChildVisit_Continue;

        CXString cxName = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxName);
        clang_disposeString(cxName);

        if (name.empty()) return CXChildVisit_Continue;
        if (name[0] == '_') return CXChildVisit_Continue;

        if (data->bindings->findGlobal(name))
            return CXChildVisit_Continue;

        CXType varType = clang_getCursorType(cursor);
        auto* varTI = data->mapper->map(varType);
        if (!varTI) return CXChildVisit_Continue;

        CGlobalVar gv;
        gv.name = name;
        gv.type = varTI;
        data->bindings->addGlobal(std::move(gv));
    }

    return CXChildVisit_Continue;
}

bool CHeaderResolver::parseHeader(const std::string& headerContent,
                                   const std::vector<std::string>& clangArgs) {
    CXIndex index = clang_createIndex(0, 0);
    if (!index) return false;

    // Build argv for clang
    std::vector<const char*> argv;
    for (auto& arg : clangArgs)
        argv.push_back(arg.c_str());

    // Create an unsaved file with the #include directive
    CXUnsavedFile unsavedFile;
    unsavedFile.Filename = "lux_include.c";
    unsavedFile.Contents = headerContent.c_str();
    unsavedFile.Length   = headerContent.size();

    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,
        "lux_include.c",
        argv.data(),
        static_cast<int>(argv.size()),
        &unsavedFile,
        1,
        CXTranslationUnit_DetailedPreprocessingRecord |
        CXTranslationUnit_SkipFunctionBodies
    );

    if (!tu) {
        clang_disposeIndex(index);
        return false;
    }

    // Check for fatal errors
    unsigned numDiags = clang_getNumDiagnostics(tu);
    bool hasFatal = false;
    for (unsigned i = 0; i < numDiags; i++) {
        CXDiagnostic diag = clang_getDiagnostic(tu, i);
        if (clang_getDiagnosticSeverity(diag) >= CXDiagnostic_Error) {
            CXString msg = clang_getDiagnosticSpelling(diag);
            std::cerr << "lux: C header error: " << clang_getCString(msg) << "\n";
            clang_disposeString(msg);
            hasFatal = true;
        }
        clang_disposeDiagnostic(diag);
    }

    if (hasFatal) {
        clang_disposeTranslationUnit(tu);
        clang_disposeIndex(index);
        return false;
    }

    // Walk the AST and extract declarations
    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

    VisitorData vdata;
    vdata.mapper   = &mapper_;
    vdata.bindings = &bindings_;

    clang_visitChildren(rootCursor, cursorVisitor, &vdata);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    return true;
}

void CHeaderResolver::detectRequiredLib(const std::string& headerName) {
    // Skip standard C library headers — they don't need -l flags
    // (libc is linked by default)
    static const std::unordered_set<std::string> stdCHeaders = {
        "stdio.h", "stdlib.h", "string.h", "math.h", "ctype.h",
        "errno.h", "signal.h", "setjmp.h", "stdarg.h", "stddef.h",
        "time.h", "limits.h", "float.h", "assert.h", "locale.h",
        "stdint.h", "stdbool.h", "inttypes.h", "complex.h",
        "tgmath.h", "fenv.h", "iso646.h", "wchar.h", "wctype.h",
        "stdalign.h", "stdatomic.h", "stdnoreturn.h", "threads.h",
        "uchar.h", "unistd.h", "fcntl.h", "sys/types.h",
        "sys/stat.h", "sys/wait.h", "sys/mman.h", "sys/socket.h",
        "netinet/in.h", "arpa/inet.h", "netdb.h", "dirent.h",
        "dlfcn.h", "poll.h", "sys/select.h", "sys/time.h",
        "sys/ioctl.h", "termios.h", "pthread.h",
    };
    if (stdCHeaders.count(headerName)) return;

    // 1) Check the well-known header table
    auto& table = knownHeaderLibs();
    auto it = table.find(headerName);
    if (it != table.end()) {
        bindings_.addRequiredLib(it->second, headerName);
        return;
    }

    // 2) Fallback: try pkg-config
    std::string flags = tryPkgConfig(headerName);
    if (!flags.empty()) {
        // pkg-config may return multiple flags like "-lfoo -lbar -L/path"
        // Split and add each -l flag individually
        std::istringstream stream(flags);
        std::string token;
        while (stream >> token) {
            if (token.rfind("-l", 0) == 0) {
                bindings_.addRequiredLib(token, headerName);
            }
        }
    }
}
