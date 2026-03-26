#include "lsp/CompletionProvider.h"
#include "lsp/ProjectContext.h"
#include "parser/Parser.h"
#include "ffi/CHeaderResolver.h"
#include "namespace/NamespaceRegistry.h"

#include <algorithm>
#include <sstream>

// ═══════════════════════════════════════════════════════════════════════
//  Static helpers
// ═══════════════════════════════════════════════════════════════════════

static bool cursorInsideNode(antlr4::ParserRuleContext* node, size_t cursorLine0) {
    if (!node) return false;
    auto* start = node->getStart();
    auto* stop  = node->getStop();
    if (!start || !stop) return false;
    size_t startLine = start->getLine() - 1;
    size_t stopLine  = stop->getLine()  - 1;
    return cursorLine0 >= startLine && cursorLine0 <= stopLine;
}

static void collectLocalsFromBlock(
    LuxParser::BlockContext* block, size_t beforeLine,
    std::unordered_map<std::string, CompletionProvider::LocalVar>& out);

static void collectLocalsFromStmts(
    const std::vector<LuxParser::StatementContext*>& stmts, size_t beforeLine,
    std::unordered_map<std::string, CompletionProvider::LocalVar>& out) {
    for (auto* stmt : stmts) {
        auto* start = stmt->getStart();
        if (start && start->getLine() > beforeLine + 1) break;

        if (auto* vd = stmt->varDeclStmt()) {
            std::string typeName;
            if (vd->typeSpec()) typeName = vd->typeSpec()->getText();
            out[vd->IDENTIFIER()->getText()] = {typeName, 0};
        }

        if (auto* ifS = stmt->ifStmt()) {
            if (cursorInsideNode(ifS->block(), beforeLine))
                collectLocalsFromBlock(ifS->block(), beforeLine, out);
            for (auto* elif : ifS->elseIfClause())
                if (cursorInsideNode(elif->block(), beforeLine))
                    collectLocalsFromBlock(elif->block(), beforeLine, out);
            if (ifS->elseClause())
                if (cursorInsideNode(ifS->elseClause()->block(), beforeLine))
                    collectLocalsFromBlock(ifS->elseClause()->block(), beforeLine, out);
        }
        if (auto* forS = stmt->forStmt()) {
            if (auto* fin = dynamic_cast<LuxParser::ForInStmtContext*>(forS)) {
                if (cursorInsideNode(fin, beforeLine)) {
                    out[fin->IDENTIFIER()->getText()] = {fin->typeSpec()->getText(), 0};
                    collectLocalsFromBlock(fin->block(), beforeLine, out);
                }
            }
            if (auto* fc = dynamic_cast<LuxParser::ForClassicStmtContext*>(forS)) {
                if (cursorInsideNode(fc, beforeLine)) {
                    out[fc->IDENTIFIER()->getText()] = {fc->typeSpec()->getText(), 0};
                    collectLocalsFromBlock(fc->block(), beforeLine, out);
                }
            }
        }
        if (auto* ws = stmt->whileStmt())
            if (cursorInsideNode(ws->block(), beforeLine))
                collectLocalsFromBlock(ws->block(), beforeLine, out);
        if (auto* dw = stmt->doWhileStmt())
            if (cursorInsideNode(dw->block(), beforeLine))
                collectLocalsFromBlock(dw->block(), beforeLine, out);
        if (auto* ls = stmt->loopStmt())
            if (cursorInsideNode(ls->block(), beforeLine))
                collectLocalsFromBlock(ls->block(), beforeLine, out);
        if (auto* sw = stmt->switchStmt()) {
            for (auto* c : sw->caseClause())
                if (cursorInsideNode(c->block(), beforeLine))
                    collectLocalsFromBlock(c->block(), beforeLine, out);
            if (sw->defaultClause())
                if (cursorInsideNode(sw->defaultClause()->block(), beforeLine))
                    collectLocalsFromBlock(sw->defaultClause()->block(), beforeLine, out);
        }
        if (auto* tc = stmt->tryCatchStmt()) {
            if (cursorInsideNode(tc->block(), beforeLine))
                collectLocalsFromBlock(tc->block(), beforeLine, out);
            for (auto* cc : tc->catchClause()) {
                if (cursorInsideNode(cc, beforeLine)) {
                    out[cc->IDENTIFIER()->getText()] = {cc->typeSpec()->getText(), 0};
                    collectLocalsFromBlock(cc->block(), beforeLine, out);
                }
            }
            if (tc->finallyClause())
                if (cursorInsideNode(tc->finallyClause()->block(), beforeLine))
                    collectLocalsFromBlock(tc->finallyClause()->block(), beforeLine, out);
        }
    }
}

static void collectLocalsFromBlock(
    LuxParser::BlockContext* block, size_t beforeLine,
    std::unordered_map<std::string, CompletionProvider::LocalVar>& out) {
    if (!block) return;
    collectLocalsFromStmts(block->statement(), beforeLine, out);
}

// ═══════════════════════════════════════════════════════════════════════
//  Static helpers for method completion (needed by collectors below)
// ═══════════════════════════════════════════════════════════════════════

// Resolve special type placeholders in method descriptors.
static std::string resolveTypePlaceholder(const std::string& raw,
                                          const std::string& selfType,
                                          const std::string& elemType,
                                          const std::string& keyType,
                                          const std::string& valType) {
    if (raw == "_self") return selfType;
    if (raw == "_elem") return elemType;
    if (raw == "_key")  return keyType.empty()  ? raw : keyType;
    if (raw == "_val")  return valType.empty()  ? raw : valType;
    return raw;
}

// Build a human-readable parameter name from its type.
static std::string paramNameFromType(const std::string& type) {
    if (type == "usize" || type == "isize") return "index";
    if (type == "bool") return "flag";
    if (type == "string") return "str";
    if (type == "char") return "ch";
    if (type == "uint32" || type == "int32") return "n";
    if (type == "float64" || type == "float32") return "x";
    if (type == "void") return "";
    // For generic/self/elem types, derive short name
    if (!type.empty() && type.front() == '[') return "arr";
    if (type.find('<') != std::string::npos) return "val";
    return "value";
}

// Build a CompletionItem from a MethodDescriptor with full signatures & snippets.
static CompletionItem buildMethodItem(const MethodDescriptor& md,
                                      const std::string& selfType,
                                      const std::string& elemType,
                                      const std::string& keyType = "",
                                      const std::string& valType = "") {
    CompletionItem ci;
    ci.label = md.name;
    ci.kind  = CompletionKind::Method;

    // Resolve return type
    std::string retType = resolveTypePlaceholder(
        md.returnType, selfType, elemType, keyType, valType);

    // Resolve parameter types
    std::vector<std::string> resolvedParams;
    resolvedParams.reserve(md.paramTypes.size());
    for (auto& pt : md.paramTypes) {
        resolvedParams.push_back(
            resolveTypePlaceholder(pt, selfType, elemType, keyType, valType));
    }

    // detail: short signature shown inline — e.g. "(usize index, int32 value) -> void"
    std::string detail = "(";
    for (size_t i = 0; i < resolvedParams.size(); i++) {
        if (i > 0) detail += ", ";
        detail += resolvedParams[i];
        std::string pname = paramNameFromType(resolvedParams[i]);
        if (!pname.empty()) detail += " " + pname;
    }
    detail += ") -> " + retType;
    ci.detail = detail;

    // documentation: markdown with full method signature
    std::string doc = "```lux\nfn " + md.name + "(";
    for (size_t i = 0; i < resolvedParams.size(); i++) {
        if (i > 0) doc += ", ";
        std::string pname = paramNameFromType(resolvedParams[i]);
        if (pname.empty()) pname = "p" + std::to_string(i);
        doc += resolvedParams[i] + " " + pname;
    }
    doc += ") -> " + retType + "\n```";
    ci.documentation = doc;

    // insertText: snippet with parameter placeholders
    if (resolvedParams.empty()) {
        ci.insertText = md.name + "()";
        ci.insertTextFormat = InsertTextFormat::PlainText;
    } else {
        std::string snippet = md.name + "(";
        for (size_t i = 0; i < resolvedParams.size(); i++) {
            if (i > 0) snippet += ", ";
            std::string pname = paramNameFromType(resolvedParams[i]);
            if (pname.empty()) pname = "p" + std::to_string(i);
            snippet += "${" + std::to_string(i + 1) + ":" + pname + "}";
        }
        snippet += ")";
        ci.insertText = snippet;
        ci.insertTextFormat = InsertTextFormat::Snippet;
    }

    // filterText: just the method name so typing filters correctly
    ci.filterText = md.name;

    return ci;
}

// ═══════════════════════════════════════════════════════════════════════
//  Public entry point
// ═══════════════════════════════════════════════════════════════════════

std::vector<CompletionItem> CompletionProvider::complete(
    const std::string& source, size_t line, size_t col,
    const std::string& filePath, const ProjectContext* project) {

    // 1) Analyze context from raw text (works even with incomplete code)
    auto req = analyzeContext(source, line, col, nullptr);

    // Short-circuit for #include header completion — no parsing needed
    if (req.context == CompletionContext::IncludeHeader) {
        std::vector<CompletionItem> items;
        addHeaderSuggestions(items, req.prefix);
        return items;
    }

    // Short-circuit for `use` import completion — no parsing needed
    if (req.context == CompletionContext::UseImport) {
        std::vector<CompletionItem> items;
        addUseCompletions(items, req.modulePath, req.prefix, project);
        return items;
    }

    // 2) Build a "patched" source for parsing: comment out the cursor line
    //    so the parser doesn't choke on incomplete expressions like "p." or "ptr->"
    std::string patchedSource;
    {
        std::istringstream ss(source);
        std::string ln;
        size_t lineIdx = 0;
        while (std::getline(ss, ln)) {
            if (lineIdx == line)
                patchedSource += "// <completion>\n";
            else
                patchedSource += ln + "\n";
            lineIdx++;
        }
    }

    auto parsed = Parser::parseString(patchedSource);
    if (!parsed.tree) return {};

    // Resolve C headers
    CBindings localBindings;
    TypeRegistry cTypeReg;
    const CBindings* cBindingsPtr = nullptr;

    if (project && project->isValid()) {
        cBindingsPtr = &project->cBindings();
    } else {
        auto includes = parsed.tree->includeDecl();
        if (!includes.empty()) {
            CHeaderResolver resolver(cTypeReg, localBindings);
            for (auto* incl : includes) {
                auto text = incl->getText();
                if (incl->INCLUDE_SYS()) {
                    auto header = CHeaderResolver::extractSystemHeader(text);
                    if (!header.empty())
                        resolver.resolveSystemHeader(header);
                } else if (incl->INCLUDE_LOCAL()) {
                    auto header = CHeaderResolver::extractLocalHeader(text);
                    if (!header.empty())
                        resolver.resolveLocalHeader(header, ".");
                }
            }
        }
        cBindingsPtr = &localBindings;
    }

    // Re-infer receiver type now that we have a valid tree
    if (req.context == CompletionContext::DotAccess ||
        req.context == CompletionContext::ArrowAccess) {
        if (req.receiverType.empty() && !req.receiverVar.empty()) {
            std::string varType = inferVarType(req.receiverVar, parsed.tree, line);
            if (req.context == CompletionContext::ArrowAccess) {
                // Strip pointer for arrow access
                if (!varType.empty() && varType[0] == '*')
                    varType = varType.substr(1);
            }

            // Unwrap array dimensions when the expression has [..] indexing
            if (req.indexDepth > 0 && !varType.empty()) {
                // First, resolve type alias to its underlying type string
                // by checking type alias declarations in the tree
                std::string resolved = varType;
                for (auto* tld : parsed.tree->topLevelDecl()) {
                    if (auto* ta = tld->typeAliasDecl()) {
                        if (ta->IDENTIFIER() &&
                            ta->IDENTIFIER()->getText() == resolved &&
                            ta->typeSpec()) {
                            resolved = ta->typeSpec()->getText();
                            break;
                        }
                    }
                }

                // Strip [N] prefixes for each index access
                unsigned depth = req.indexDepth;
                while (depth > 0 && resolved.size() > 2 && resolved[0] == '[') {
                    auto closeBracket = resolved.find(']');
                    if (closeBracket != std::string::npos &&
                        closeBracket + 1 <= resolved.size()) {
                        resolved = resolved.substr(closeBracket + 1);
                        --depth;
                    } else {
                        break;
                    }
                }
                varType = resolved;
            }

            // Resolve method chain: x.abs().toString().| → walk each call
            if (!req.methodChain.empty() && !varType.empty()) {
                for (auto& methodName : req.methodChain) {
                    std::string retType = resolveMethodReturnType(
                        varType, methodName, parsed.tree, project);
                    if (retType.empty()) break; // unknown method, stop
                    varType = retType;
                }
            }

            req.receiverType = varType;
        }
    }
    std::vector<CompletionItem> items;

    switch (req.context) {
        case CompletionContext::DotAccess:
        case CompletionContext::ArrowAccess: {
            addStructFields(items, req.receiverType, parsed.tree,
                            *cBindingsPtr, project);
            addExtendMethods(items, req.receiverType, parsed.tree, project);
            addTypeMethods(items, req.receiverType, req.prefix);
            break;
        }
        case CompletionContext::ScopeAccess: {
            addEnumVariants(items, req.scopeName, parsed.tree,
                            *cBindingsPtr, project);
            addStaticMethods(items, req.scopeName, parsed.tree, project);
            break;
        }
        case CompletionContext::TypePosition: {
            addTypeNames(items, parsed.tree, *cBindingsPtr, project, req.prefix);
            break;
        }
        case CompletionContext::General: {
            addLocals(items, parsed.tree, line, req.prefix);
            addLocalDecls(items, parsed.tree, req.prefix);
            addImportedSymbols(items, parsed.tree, project, req.prefix);
            addGlobalBuiltins(items, req.prefix);
            addCSymbols(items, *cBindingsPtr, req.prefix);
            addKeywords(items, req.prefix);
            addTypeNames(items, parsed.tree, *cBindingsPtr, project, req.prefix);
            break;
        }
    }

    dedup(items);
    return items;
}

// ═══════════════════════════════════════════════════════════════════════
//  Context analysis (text-based, pre-parse)
// ═══════════════════════════════════════════════════════════════════════

CompletionProvider::CompletionRequest CompletionProvider::analyzeContext(
    const std::string& source, size_t line, size_t col,
    LuxParser::ProgramContext* tree) {

    CompletionRequest req;

    // Get the current line text
    std::istringstream stream(source);
    std::string lineText;
    for (size_t i = 0; i <= line; i++) {
        if (!std::getline(stream, lineText)) {
            lineText.clear();
            break;
        }
    }

    // Clamp col
    if (col > lineText.size()) col = lineText.size();

    // Extract the text before cursor on this line
    std::string before = lineText.substr(0, col);

    // Check for #include directive completion
    {
        std::string trimmed = before;
        size_t ws = trimmed.find_first_not_of(" \t");
        if (ws != std::string::npos) trimmed = trimmed.substr(ws);

        // Match: #include <prefix
        if (trimmed.size() >= 10 && trimmed.substr(0, 10) == "#include <") {
            req.context = CompletionContext::IncludeHeader;
            req.prefix = trimmed.substr(10); // everything after '<'
            return req;
        }
        // Match: #include "prefix (local headers — same logic)
        if (trimmed.size() >= 10 && trimmed.substr(0, 10) == "#include \"") {
            req.context = CompletionContext::IncludeHeader;
            req.prefix = trimmed.substr(10);
            return req;
        }
    }

    // Check for `use` import path completion
    {
        std::string trimmed = before;
        size_t ws = trimmed.find_first_not_of(" \t");
        if (ws != std::string::npos) trimmed = trimmed.substr(ws);

        // Detect: use ...
        bool isUse = (trimmed.substr(0, 4) == "use " && trimmed.size() >= 4);
        if (isUse) {
            std::string afterUse = trimmed.substr(4);
            // Strip leading whitespace
            size_t start = afterUse.find_first_not_of(" \t");
            if (start != std::string::npos)
                afterUse = afterUse.substr(start);
            else
                afterUse.clear();

            // Check for group import: use std::log::{println, |
            // Find last '{' — if present, we're completing inside a group
            auto bracePos = afterUse.rfind('{');
            if (bracePos != std::string::npos) {
                // Extract module path before ::{ 
                std::string pathBeforeBrace = afterUse.substr(0, bracePos);
                // Remove trailing ::
                while (pathBeforeBrace.size() >= 2 &&
                       pathBeforeBrace.back() == ':' &&
                       pathBeforeBrace[pathBeforeBrace.size()-2] == ':')
                    pathBeforeBrace.resize(pathBeforeBrace.size() - 2);
                // strip trailing whitespace
                while (!pathBeforeBrace.empty() && pathBeforeBrace.back() == ' ')
                    pathBeforeBrace.pop_back();

                // Extract prefix: text after last comma or brace
                std::string insideBrace = afterUse.substr(bracePos + 1);
                auto lastComma = insideBrace.rfind(',');
                std::string partial;
                if (lastComma != std::string::npos)
                    partial = insideBrace.substr(lastComma + 1);
                else
                    partial = insideBrace;
                // trim spaces
                size_t pws = partial.find_first_not_of(" \t");
                if (pws != std::string::npos) partial = partial.substr(pws);
                else partial.clear();

                req.context = CompletionContext::UseImport;
                req.modulePath = pathBeforeBrace;
                req.prefix = partial;
                return req;
            }

            // Not inside braces — check if cursor is right after ::
            // Find the last :: to split path vs prefix
            auto lastScope = afterUse.rfind("::");
            if (lastScope != std::string::npos) {
                std::string pathPart = afterUse.substr(0, lastScope);
                std::string partial = afterUse.substr(lastScope + 2);
                // trim partial
                while (!partial.empty() && partial.back() == ' ') partial.pop_back();

                req.context = CompletionContext::UseImport;
                req.modulePath = pathPart;
                req.prefix = partial;
                return req;
            }

            // Just "use " or "use st" — completing the root module name  
            req.context = CompletionContext::UseImport;
            req.modulePath.clear();
            req.prefix = afterUse;
            // trim trailing
            while (!req.prefix.empty() && req.prefix.back() == ' ') req.prefix.pop_back();
            return req;
        }
    }

    // Check for scope access: Name::|
    {
        // Look for IDENTIFIER:: just before cursor
        size_t pos = before.size();
        if (pos >= 2 && before[pos - 1] == ':' && before[pos - 2] == ':') {
            // Extract identifier before ::
            size_t end = pos - 2;
            size_t start = end;
            while (start > 0 && (std::isalnum(before[start - 1]) || before[start - 1] == '_'))
                --start;
            if (start < end) {
                req.context = CompletionContext::ScopeAccess;
                req.scopeName = before.substr(start, end - start);
                req.prefix.clear();
                return req;
            }
        }
        // Also handle Name::partial|
        if (pos >= 3) {
            size_t idEnd = pos;
            while (idEnd > 0 && (std::isalnum(before[idEnd - 1]) || before[idEnd - 1] == '_'))
                --idEnd;
            if (idEnd >= 2 && before[idEnd - 1] == ':' && before[idEnd - 2] == ':') {
                size_t scopeEnd = idEnd - 2;
                size_t scopeStart = scopeEnd;
                while (scopeStart > 0 && (std::isalnum(before[scopeStart - 1]) || before[scopeStart - 1] == '_'))
                    --scopeStart;
                if (scopeStart < scopeEnd) {
                    req.context = CompletionContext::ScopeAccess;
                    req.scopeName = before.substr(scopeStart, scopeEnd - scopeStart);
                    req.prefix = before.substr(idEnd);
                    return req;
                }
            }
        }
    }

    // Check for arrow access: expr->| or expr->partial|
    {
        size_t pos = before.size();
        // First strip any partial identifier
        size_t idEnd = pos;
        while (idEnd > 0 && (std::isalnum(before[idEnd - 1]) || before[idEnd - 1] == '_'))
            --idEnd;

        if (idEnd >= 2 && before[idEnd - 1] == '>' && before[idEnd - 2] == '-') {
            req.prefix = before.substr(idEnd);
            // Extract receiver variable name
            size_t arrowPos = idEnd - 2;
            size_t recEnd = arrowPos;
            while (recEnd > 0 && (std::isalnum(before[recEnd - 1]) || before[recEnd - 1] == '_'))
                --recEnd;
            req.receiverVar = before.substr(recEnd, arrowPos - recEnd);
            req.context = CompletionContext::ArrowAccess;
            return req;
        }
    }

    // Check for dot access: expr.| or expr.partial| or expr[...].| or expr.method().| chains
    {
        size_t pos = before.size();
        size_t idEnd = pos;
        while (idEnd > 0 && (std::isalnum(before[idEnd - 1]) || before[idEnd - 1] == '_'))
            --idEnd;

        if (idEnd > 0 && before[idEnd - 1] == '.') {
            req.prefix = before.substr(idEnd);
            size_t dotPos = idEnd - 1;
            size_t recEnd = dotPos;

            // Walk backwards through the expression, collecting method chain
            // Patterns to handle:
            //   .name(...)   → method call, record name in chain
            //   [...]        → array index, increment indexDepth
            //   identifier   → base variable, stop
            std::vector<std::string> chain;
            unsigned indexDepth = 0;

            while (recEnd > 0) {
                // Try: ...method_name(...)
                if (before[recEnd - 1] == ')') {
                    // Walk back over balanced (...)
                    size_t parenEnd = recEnd - 1;
                    int depth = 1;
                    size_t p = parenEnd;
                    while (p > 0 && depth > 0) {
                        --p;
                        if (before[p] == ')') ++depth;
                        else if (before[p] == '(') --depth;
                    }
                    if (depth != 0) break; // unbalanced
                    // p is now at '(', walk back to get method name
                    size_t nameEnd = p;
                    size_t nameStart = nameEnd;
                    while (nameStart > 0 && (std::isalnum(before[nameStart - 1]) || before[nameStart - 1] == '_'))
                        --nameStart;
                    if (nameStart == nameEnd) break; // no method name before '('
                    std::string methodName = before.substr(nameStart, nameEnd - nameStart);
                    chain.push_back(methodName);
                    recEnd = nameStart;
                    // Check for '.' before the method name
                    if (recEnd > 0 && before[recEnd - 1] == '.')
                        --recEnd; // skip the dot
                    else
                        break; // no dot means this is a function call, not a chain
                }
                // Try: ...[...]
                else if (before[recEnd - 1] == ']') {
                    size_t bracketEnd = recEnd - 1;
                    int depth = 1;
                    size_t p = bracketEnd;
                    while (p > 0 && depth > 0) {
                        --p;
                        if (before[p] == ']') ++depth;
                        else if (before[p] == '[') --depth;
                    }
                    if (depth != 0) break;
                    recEnd = p;
                    ++indexDepth;
                }
                // Otherwise: base variable identifier
                else {
                    break;
                }
            }

            // Extract the base variable name
            size_t nameEnd = recEnd;
            while (recEnd > 0 && (std::isalnum(before[recEnd - 1]) || before[recEnd - 1] == '_'))
                --recEnd;
            req.receiverVar = before.substr(recEnd, nameEnd - recEnd);
            req.indexDepth = indexDepth;
            // Reverse chain since we collected from right to left
            std::reverse(chain.begin(), chain.end());
            req.methodChain = std::move(chain);
            req.context = CompletionContext::DotAccess;
            return req;
        }
    }

    // Check for type position — heuristics:
    // After keywords that expect a type: let, var, fn params, return type
    // Simple heuristic: if line matches common type-position patterns
    {
        // Trim leading whitespace
        std::string trimmed = before;
        size_t ws = trimmed.find_first_not_of(" \t");
        if (ws != std::string::npos) trimmed = trimmed.substr(ws);

        // After ":" in struct literal field or after cast "as "
        if (trimmed.size() >= 3 && trimmed.substr(trimmed.size() - 3) == "as ") {
            req.context = CompletionContext::TypePosition;
            req.prefix.clear();
            return req;
        }
    }

    // General completion: extract prefix
    {
        size_t pos = col;
        while (pos > 0 && (std::isalnum(before[pos - 1]) || before[pos - 1] == '_'))
            --pos;
        req.prefix = before.substr(pos);
        req.context = CompletionContext::General;
    }

    return req;
}

// ═══════════════════════════════════════════════════════════════════════
//  Collectors
// ═══════════════════════════════════════════════════════════════════════

void CompletionProvider::addLocals(std::vector<CompletionItem>& items,
                                   LuxParser::ProgramContext* tree,
                                   size_t cursorLine,
                                   const std::string& prefix) {
    // Check if inside a function
    auto* func = findEnclosingFunction(tree, cursorLine);
    if (func) {
        auto locals = collectLocals(func, cursorLine);
        for (auto& [name, var] : locals) {
            if (!matchesPrefix(name, prefix)) continue;
            CompletionItem item;
            item.label = name;
            item.kind = CompletionKind::Variable;
            item.detail = var.typeName;
            items.push_back(std::move(item));
        }

        // Also add function parameters
        if (auto* params = func->paramList()) {
            for (auto* p : params->param()) {
                std::string name = p->IDENTIFIER()->getText();
                if (!matchesPrefix(name, prefix)) continue;
                CompletionItem item;
                item.label = name;
                item.kind = CompletionKind::Variable;
                item.detail = typeSpecToString(p->typeSpec());
                items.push_back(std::move(item));
            }
        }
        return;
    }

    // Check if inside an extend method
    auto* method = findEnclosingMethod(tree, cursorLine);
    if (method) {
        auto locals = collectLocalsFromMethod(method, cursorLine);
        for (auto& [name, var] : locals) {
            if (!matchesPrefix(name, prefix)) continue;
            CompletionItem item;
            item.label = name;
            item.kind = CompletionKind::Variable;
            item.detail = var.typeName;
            items.push_back(std::move(item));
        }
    }
}

void CompletionProvider::addLocalDecls(std::vector<CompletionItem>& items,
                                       LuxParser::ProgramContext* tree,
                                       const std::string& prefix) {
    for (auto* tld : tree->topLevelDecl()) {
        // Functions
        if (auto* func = tld->functionDecl()) {
            std::string name = func->IDENTIFIER()->getText();
            if (!matchesPrefix(name, prefix)) continue;
            CompletionItem item;
            item.label = name;
            item.kind = CompletionKind::Function;
            item.detail = formatFuncSignature(func);

            // Build snippet with parameter placeholders
            if (auto* params = func->paramList(); params && !params->param().empty()) {
                std::string snippet = name + "(";
                auto paramList = params->param();
                for (size_t i = 0; i < paramList.size(); i++) {
                    if (i > 0) snippet += ", ";
                    snippet += "${" + std::to_string(i + 1) + ":" +
                               paramList[i]->IDENTIFIER()->getText() + "}";
                }
                snippet += ")";
                item.insertText = snippet;
                item.insertTextFormat = InsertTextFormat::Snippet;
            } else {
                item.insertText = name + "()";
            }
            // Documentation with full signature
            item.documentation = "```lux\n" + item.detail + "\n```";
            items.push_back(std::move(item));
        }

        // Structs
        if (auto* sd = tld->structDecl()) {
            std::string name = sd->IDENTIFIER()->getText();
            if (!matchesPrefix(name, prefix)) continue;
            CompletionItem item;
            item.label = name;
            item.kind = CompletionKind::Struct;
            item.detail = "struct " + name;
            items.push_back(std::move(item));
        }

        // Enums
        if (auto* ed = tld->enumDecl()) {
            auto ids = ed->IDENTIFIER();
            if (ids.empty()) continue;
            std::string name = ids[0]->getText();
            if (!matchesPrefix(name, prefix)) continue;
            CompletionItem item;
            item.label = name;
            item.kind = CompletionKind::Enum;
            item.detail = "enum " + name;
            items.push_back(std::move(item));
        }

        // Unions
        if (auto* ud = tld->unionDecl()) {
            std::string name = ud->IDENTIFIER()->getText();
            if (!matchesPrefix(name, prefix)) continue;
            CompletionItem item;
            item.label = name;
            item.kind = CompletionKind::Struct;
            item.detail = "union " + name;
            items.push_back(std::move(item));
        }

        // Type aliases
        if (auto* ta = tld->typeAliasDecl()) {
            std::string name = ta->IDENTIFIER()->getText();
            if (!matchesPrefix(name, prefix)) continue;
            CompletionItem item;
            item.label = name;
            item.kind = CompletionKind::Class;
            item.detail = "type " + name + " = " + typeSpecToString(ta->typeSpec());
            items.push_back(std::move(item));
        }

        // Extern functions
        if (auto* ext = tld->externDecl()) {
            std::string name = ext->IDENTIFIER()->getText();
            if (!matchesPrefix(name, prefix)) continue;
            CompletionItem item;
            item.label = name;
            item.kind = CompletionKind::Function;
            std::string detail = typeSpecToString(ext->typeSpec()) + " " + name + "(";
            if (auto* params = ext->externParamList()) {
                bool first = true;
                for (auto* p : params->externParam()) {
                    if (!first) detail += ", ";
                    first = false;
                    detail += typeSpecToString(p->typeSpec());
                    if (p->IDENTIFIER()) detail += " " + p->IDENTIFIER()->getText();
                }
            }
            if (ext->SPREAD()) {
                if (ext->externParamList()) detail += ", ";
                detail += "...";
            }
            detail += ")";
            item.detail = detail;

            // Build snippet for extern function
            if (auto* params = ext->externParamList(); params && !params->externParam().empty()) {
                std::string snippet = name + "(";
                auto paramList = params->externParam();
                for (size_t i = 0; i < paramList.size(); i++) {
                    if (i > 0) snippet += ", ";
                    std::string pname = paramList[i]->IDENTIFIER()
                        ? paramList[i]->IDENTIFIER()->getText()
                        : "p" + std::to_string(i);
                    snippet += "${" + std::to_string(i + 1) + ":" + pname + "}";
                }
                snippet += ")";
                item.insertText = snippet;
                item.insertTextFormat = InsertTextFormat::Snippet;
            } else {
                item.insertText = name + "()";
            }
            item.documentation = "```lux\n" + detail + "\n```";
            items.push_back(std::move(item));
        }
    }
}

void CompletionProvider::addImportedSymbols(std::vector<CompletionItem>& items,
                                            LuxParser::ProgramContext* tree,
                                            const ProjectContext* project,
                                            const std::string& prefix) {
    if (!project || !project->isValid()) return;

    for (auto* useDecl : tree->useDecl()) {
        std::string modulePath;
        std::vector<std::string> symbolNames;

        if (auto* item = dynamic_cast<LuxParser::UseItemContext*>(useDecl)) {
            if (!item->modulePath() || !item->IDENTIFIER()) continue;
            for (auto* id : item->modulePath()->IDENTIFIER()) {
                if (!modulePath.empty()) modulePath += "::";
                modulePath += id->getText();
            }
            symbolNames.push_back(item->IDENTIFIER()->getText());
        } else if (auto* group = dynamic_cast<LuxParser::UseGroupContext*>(useDecl)) {
            if (!group->modulePath()) continue;
            for (auto* id : group->modulePath()->IDENTIFIER()) {
                if (!modulePath.empty()) modulePath += "::";
                modulePath += id->getText();
            }
            for (auto* id : group->IDENTIFIER()) {
                symbolNames.push_back(id->getText());
            }
        }

        for (auto& symName : symbolNames) {
            if (!matchesPrefix(symName, prefix)) continue;

            auto* sym = project->registry().findSymbol(modulePath, symName);

            if (!sym) {
                // Fallback: check stdlib builtins for std:: modules
                if (modulePath.rfind("std::", 0) == 0) {
                    // Check if it's a builtin function
                    const auto* sig = builtinRegistry_.lookup(symName);
                    if (sig) {
                        CompletionItem ci;
                        ci.label = symName;
                        ci.kind = CompletionKind::Function;

                        // Build detail: (paramTypes) -> returnType
                        std::string detail = "(";
                        for (size_t i = 0; i < sig->paramTypes.size(); i++) {
                            if (i > 0) detail += ", ";
                            detail += sig->paramTypes[i];
                        }
                        if (sig->isVariadic) {
                            if (!sig->paramTypes.empty()) detail += ", ";
                            detail += "...";
                        }
                        detail += ") -> " + sig->returnType;
                        ci.detail = detail;

                        ci.documentation = "```lux\nfn " + symName + detail + "\n```\n\nImported from `" + modulePath + "`";

                        // Build snippet with parameter placeholders
                        if (!sig->paramTypes.empty()) {
                            std::string snippet = symName + "(";
                            for (size_t i = 0; i < sig->paramTypes.size(); i++) {
                                if (i > 0) snippet += ", ";
                                snippet += "${" + std::to_string(i + 1) + ":" +
                                           paramNameFromType(sig->paramTypes[i]) + "}";
                            }
                            if (sig->isVariadic) {
                                snippet += ", ${" + std::to_string(sig->paramTypes.size() + 1) + ":...}";
                            }
                            snippet += ")";
                            ci.insertText = snippet;
                            ci.insertTextFormat = InsertTextFormat::Snippet;
                        } else if (sig->isVariadic) {
                            ci.insertText = symName + "(${1:...})";
                            ci.insertTextFormat = InsertTextFormat::Snippet;
                        } else {
                            ci.insertText = symName + "()";
                        }

                        items.push_back(std::move(ci));
                        continue;
                    }

                    // Check if it's a builtin constant (e.g. PI, E)
                    const auto& constType = builtinRegistry_.lookupConstant(symName);
                    if (!constType.empty()) {
                        CompletionItem ci;
                        ci.label = symName;
                        ci.kind = CompletionKind::Constant;
                        ci.detail = constType;
                        ci.documentation = "```lux\nconst " + symName + ": " + constType + "\n```\n\nImported from `" + modulePath + "`";
                        items.push_back(std::move(ci));
                        continue;
                    }
                }
                continue;
            }

            CompletionItem ci;
            ci.label = symName;

            switch (sym->kind) {
                case ExportedSymbol::Function: {
                    ci.kind = CompletionKind::Function;
                    auto* fd = dynamic_cast<LuxParser::FunctionDeclContext*>(sym->decl);
                    if (fd) {
                        ci.detail = formatFuncSignature(fd);
                        ci.documentation = "```lux\n" + ci.detail + "\n```";
                        // Build snippet with parameter placeholders
                        if (auto* params = fd->paramList();
                            params && !params->param().empty()) {
                            std::string snippet = symName + "(";
                            auto pList = params->param();
                            for (size_t i = 0; i < pList.size(); i++) {
                                if (i > 0) snippet += ", ";
                                snippet += "${" + std::to_string(i + 1) + ":" +
                                           pList[i]->IDENTIFIER()->getText() + "}";
                            }
                            snippet += ")";
                            ci.insertText = snippet;
                            ci.insertTextFormat = InsertTextFormat::Snippet;
                        } else {
                            ci.insertText = symName + "()";
                        }
                    }
                    break;
                }
                case ExportedSymbol::Struct:
                    ci.kind = CompletionKind::Struct;
                    ci.detail = "struct " + symName;
                    break;
                case ExportedSymbol::Enum:
                    ci.kind = CompletionKind::Enum;
                    ci.detail = "enum " + symName;
                    break;
                case ExportedSymbol::Union:
                    ci.kind = CompletionKind::Struct;
                    ci.detail = "union " + symName;
                    break;
                case ExportedSymbol::TypeAlias:
                    ci.kind = CompletionKind::Class;
                    ci.detail = "type " + symName;
                    break;
                case ExportedSymbol::ExtendBlock:
                    continue; // extend blocks not directly completable
            }

            items.push_back(std::move(ci));
        }
    }
}

void CompletionProvider::addProjectSymbols(std::vector<CompletionItem>& items,
                                           const ProjectContext* project,
                                           const std::string& filePath,
                                           const std::string& prefix) {
    if (!project || !project->isValid()) return;

    std::string currentNs = project->namespaceFor(filePath);

    for (auto& ns : project->registry().allNamespaces()) {
        if (ns == currentNs) continue; // skip same-file symbols
        auto syms = project->registry().getNamespaceSymbols(ns);
        for (auto* sym : syms) {
            if (sym->kind == ExportedSymbol::ExtendBlock) continue;
            if (!matchesPrefix(sym->name, prefix)) continue;

            CompletionItem ci;
            ci.label = sym->name;
            ci.detail = "[" + ns + "]";

            switch (sym->kind) {
                case ExportedSymbol::Function:
                    ci.kind = CompletionKind::Function;
                    break;
                case ExportedSymbol::Struct:
                    ci.kind = CompletionKind::Struct;
                    break;
                case ExportedSymbol::Enum:
                    ci.kind = CompletionKind::Enum;
                    break;
                case ExportedSymbol::Union:
                    ci.kind = CompletionKind::Struct;
                    break;
                case ExportedSymbol::TypeAlias:
                    ci.kind = CompletionKind::Class;
                    break;
                default:
                    continue;
            }

            items.push_back(std::move(ci));
        }
    }
}

void CompletionProvider::addCSymbols(std::vector<CompletionItem>& items,
                                     const CBindings& bindings,
                                     const std::string& prefix) {
    // C functions
    for (auto& [name, cf] : bindings.functions()) {
        if (!matchesPrefix(name, prefix)) continue;
        CompletionItem ci;
        ci.label = name;
        ci.kind = CompletionKind::Function;
        std::string detail = "(C) ";
        if (cf.returnType) detail += cf.returnType->name;
        detail += " " + name + "(";
        for (size_t i = 0; i < cf.paramTypes.size(); i++) {
            if (i > 0) detail += ", ";
            if (cf.paramTypes[i]) detail += cf.paramTypes[i]->name;
        }
        if (cf.isVariadic) {
            if (!cf.paramTypes.empty()) detail += ", ";
            detail += "...";
        }
        detail += ")";
        ci.detail = detail;
        ci.documentation = "```c\n" + detail.substr(4) + "\n```"; // strip "(C) "
        // Snippet for C functions
        if (!cf.paramTypes.empty()) {
            std::string snippet = name + "(";
            for (size_t i = 0; i < cf.paramTypes.size(); i++) {
                if (i > 0) snippet += ", ";
                std::string pname = cf.paramTypes[i]
                    ? paramNameFromType(cf.paramTypes[i]->name)
                    : "p" + std::to_string(i);
                if (pname.empty()) pname = "p" + std::to_string(i);
                snippet += "${" + std::to_string(i + 1) + ":" + pname + "}";
            }
            snippet += ")";
            ci.insertText = snippet;
            ci.insertTextFormat = InsertTextFormat::Snippet;
        } else {
            ci.insertText = name + "()";
        }
        items.push_back(std::move(ci));
    }

    // C structs
    for (auto& [name, cs] : bindings.structs()) {
        if (!matchesPrefix(name, prefix)) continue;
        CompletionItem ci;
        ci.label = name;
        ci.kind = CompletionKind::Struct;
        ci.detail = "(C) struct " + name;
        items.push_back(std::move(ci));
    }

    // C enums
    for (auto& [name, ce] : bindings.enums()) {
        if (!matchesPrefix(name, prefix)) continue;
        CompletionItem ci;
        ci.label = name;
        ci.kind = CompletionKind::Enum;
        ci.detail = "(C) enum " + name;
        items.push_back(std::move(ci));
    }

    // C macros
    for (auto& [name, cm] : bindings.macros()) {
        if (!matchesPrefix(name, prefix)) continue;
        CompletionItem ci;
        ci.label = name;
        ci.kind = CompletionKind::Constant;
        ci.detail = "(C) #define " + name;
        items.push_back(std::move(ci));
    }

    // C struct macros
    for (auto& [name, sm] : bindings.structMacros()) {
        if (!matchesPrefix(name, prefix)) continue;
        CompletionItem ci;
        ci.label = name;
        ci.kind = CompletionKind::Constant;
        ci.detail = "(C) " + sm.structType + " " + name;
        items.push_back(std::move(ci));
    }

    // C typedefs
    for (auto& [name, ct] : bindings.typedefs()) {
        if (!matchesPrefix(name, prefix)) continue;
        CompletionItem ci;
        ci.label = name;
        ci.kind = CompletionKind::Class;
        ci.detail = "(C) typedef " + name;
        items.push_back(std::move(ci));
    }

    // C globals
    for (auto& [name, gv] : bindings.globals()) {
        if (!matchesPrefix(name, prefix)) continue;
        CompletionItem ci;
        ci.label = name;
        ci.kind = CompletionKind::Variable;
        ci.detail = "(C) ";
        if (gv.type) ci.detail += gv.type->name;
        ci.detail += " " + name;
        items.push_back(std::move(ci));
    }

    // C enum constants (flat — not behind EnumName::)
    for (auto& [ename, ce] : bindings.enums()) {
        for (auto& [valName, val] : ce.values) {
            if (!matchesPrefix(valName, prefix)) continue;
            CompletionItem ci;
            ci.label = valName;
            ci.kind = CompletionKind::EnumMember;
            ci.detail = "(C) " + ename + " = " + std::to_string(val);
            items.push_back(std::move(ci));
        }
    }
}

void CompletionProvider::addStructFields(std::vector<CompletionItem>& items,
                                         const std::string& structName,
                                         LuxParser::ProgramContext* tree,
                                         const CBindings& bindings,
                                         const ProjectContext* project) {
    if (structName.empty()) return;

    // Same-file struct
    auto* sd = findStructDecl(tree, structName);
    if (sd) {
        for (auto* field : sd->structField()) {
            CompletionItem ci;
            ci.label = field->IDENTIFIER()->getText();
            ci.kind = CompletionKind::Field;
            ci.detail = typeSpecToString(field->typeSpec());
            items.push_back(std::move(ci));
        }
        return;
    }

    // Cross-file struct
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto* sym = project->registry().findSymbol(ns, structName);
            if (!sym || sym->kind != ExportedSymbol::Struct) continue;
            auto* decl = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl);
            if (!decl) continue;
            for (auto* field : decl->structField()) {
                CompletionItem ci;
                ci.label = field->IDENTIFIER()->getText();
                ci.kind = CompletionKind::Field;
                ci.detail = typeSpecToString(field->typeSpec());
                items.push_back(std::move(ci));
            }
            return;
        }
    }

    // C struct
    auto* cs = bindings.findStruct(structName);
    if (cs) {
        for (auto& field : cs->fields) {
            CompletionItem ci;
            ci.label = field.name;
            ci.kind = CompletionKind::Field;
            if (field.typeInfo) ci.detail = field.typeInfo->name;
            items.push_back(std::move(ci));
        }
    }
}

void CompletionProvider::addExtendMethods(std::vector<CompletionItem>& items,
                                          const std::string& typeName,
                                          LuxParser::ProgramContext* tree,
                                          const ProjectContext* project) {
    if (typeName.empty()) return;

    // Same-file extend blocks
    for (auto* tld : tree->topLevelDecl()) {
        auto* ext = tld->extendDecl();
        if (!ext || ext->IDENTIFIER()->getText() != typeName) continue;
        for (auto* m : ext->extendMethod()) {
            // Skip static methods (no &self parameter)
            bool isInstance = (m->AMPERSAND() != nullptr);
            if (!isInstance) continue;
            CompletionItem ci;
            std::string mName = m->IDENTIFIER(0)->getText();
            ci.label = mName;
            ci.kind = CompletionKind::Method;
            ci.detail = formatMethodSignature(m);
            ci.documentation = "```lux\n" + ci.detail + "\n```";
            // Snippet with parameter placeholder for instance method params
            auto methodParams = m->param();
            if (!methodParams.empty()) {
                std::string snippet = mName + "(";
                for (size_t i = 0; i < methodParams.size(); i++) {
                    if (i > 0) snippet += ", ";
                    snippet += "${" + std::to_string(i + 1) + ":" +
                               methodParams[i]->IDENTIFIER()->getText() + "}";
                }
                snippet += ")";
                ci.insertText = snippet;
                ci.insertTextFormat = InsertTextFormat::Snippet;
            } else {
                ci.insertText = mName + "()";
            }
            items.push_back(std::move(ci));
        }
    }

    // Cross-file extend blocks
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto syms = project->registry().getNamespaceSymbols(ns);
            for (auto* sym : syms) {
                if (sym->kind != ExportedSymbol::ExtendBlock) continue;
                auto* ext = dynamic_cast<LuxParser::ExtendDeclContext*>(sym->decl);
                if (!ext || ext->IDENTIFIER()->getText() != typeName) continue;
                for (auto* m : ext->extendMethod()) {
                    bool isInstance = (m->AMPERSAND() != nullptr);
                    if (!isInstance) continue;
                    CompletionItem ci;
                    std::string mName = m->IDENTIFIER(0)->getText();
                    ci.label = mName;
                    ci.kind = CompletionKind::Method;
                    ci.detail = formatMethodSignature(m);
                    ci.documentation = "```lux\n" + ci.detail + "\n```";
                    auto methodParams = m->param();
                    if (!methodParams.empty()) {
                        std::string snippet = mName + "(";
                        for (size_t i = 0; i < methodParams.size(); i++) {
                            if (i > 0) snippet += ", ";
                            snippet += "${" + std::to_string(i + 1) + ":" +
                                       methodParams[i]->IDENTIFIER()->getText() + "}";
                        }
                        snippet += ")";
                        ci.insertText = snippet;
                        ci.insertTextFormat = InsertTextFormat::Snippet;
                    } else {
                        ci.insertText = mName + "()";
                    }
                    items.push_back(std::move(ci));
                }
            }
        }
    }
}

// ─── Built-in type methods (int.abs, string.len, vec.push, etc.) ────

void CompletionProvider::addTypeMethods(std::vector<CompletionItem>& items,
                                        const std::string& typeName,
                                        const std::string& prefix) {
    if (typeName.empty()) return;

    // 1) Check for array type: []T or [N]T
    bool isArray = false;
    std::string elemTypeName;
    if (typeName.size() > 2 && typeName[0] == '[') {
        isArray = true;
        auto closeBracket = typeName.find(']');
        if (closeBracket != std::string::npos && closeBracket + 1 < typeName.size())
            elemTypeName = typeName.substr(closeBracket + 1);
    }

    if (isArray) {
        auto arrayMethods = methodRegistry_.arrayMethods();
        for (auto* md : arrayMethods) {
            if (!matchesPrefix(md->name, prefix)) continue;
            if (md->requireNumeric) {
                auto* elemTI = typeRegistry_.lookup(elemTypeName);
                if (!elemTI || (elemTI->kind != TypeKind::Integer &&
                                elemTI->kind != TypeKind::Float))
                    continue;
            }
            items.push_back(buildMethodItem(*md, typeName, elemTypeName));
        }
        return;
    }

    // 2) Check for extended type: Vec<T>, Map<K,V>, Set<T>
    auto angleBracket = typeName.find('<');
    if (angleBracket != std::string::npos) {
        std::string baseName = typeName.substr(0, angleBracket);
        std::string innerType;
        std::string keyType, valType;
        if (angleBracket + 1 < typeName.size()) {
            auto closeAngle = typeName.rfind('>');
            if (closeAngle != std::string::npos && closeAngle > angleBracket)
                innerType = typeName.substr(angleBracket + 1,
                                            closeAngle - angleBracket - 1);
        }
        // For Map<K,V>, split inner on comma
        auto comma = innerType.find(',');
        if (comma != std::string::npos) {
            keyType = innerType.substr(0, comma);
            valType = innerType.substr(comma + 1);
            // Trim whitespace
            while (!valType.empty() && valType[0] == ' ') valType.erase(0, 1);
        } else {
            // Vec<T> / Set<T> — element type is the inner
            keyType = innerType;
            valType = innerType;
        }

        auto* extDesc = extTypeRegistry_.lookup(baseName);
        if (extDesc) {
            for (auto& md : extDesc->methods) {
                if (!matchesPrefix(md.name, prefix)) continue;
                if (md.requireNumeric) {
                    auto* elemTI = typeRegistry_.lookup(innerType);
                    if (!elemTI || (elemTI->kind != TypeKind::Integer &&
                                    elemTI->kind != TypeKind::Float))
                        continue;
                }
                items.push_back(
                    buildMethodItem(md, typeName, innerType, keyType, valType));
            }
            return;
        }
    }

    // 3) Built-in primitive type methods via TypeRegistry + MethodRegistry
    auto* typeInfo = typeRegistry_.lookup(typeName);
    if (!typeInfo) return;

    auto methods = methodRegistry_.methodsFor(typeInfo->kind);
    for (auto* md : methods) {
        if (!matchesPrefix(md->name, prefix)) continue;
        if (md->requireSigned && typeInfo->kind == TypeKind::Integer &&
            !typeInfo->isSigned)
            continue;
        if (md->requireUnsigned && typeInfo->kind == TypeKind::Integer &&
            typeInfo->isSigned)
            continue;
        items.push_back(buildMethodItem(*md, typeName, ""));
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Use import path completions
// ═══════════════════════════════════════════════════════════════════════

void CompletionProvider::addUseCompletions(std::vector<CompletionItem>& items,
                                           const std::string& modulePath,
                                           const std::string& prefix,
                                           const ProjectContext* project) {

    // Known std modules and their symbols (from ImportResolver)
    static const std::unordered_map<std::string, std::vector<std::string>> stdModules = {
        {"std::log", {"println", "print", "eprint", "eprintln", "dbg", "sprintf"}},
        {"std::io", {"readLine", "readChar", "readInt", "readFloat", "readBool",
                     "readAll", "prompt", "promptInt", "promptFloat", "promptBool",
                     "readByte", "readLines", "readNBytes", "isEOF",
                     "readPassword", "promptPassword", "flush", "flushErr",
                     "isTTY", "isStdoutTTY", "isStderrTTY"}},
        {"std::math", {"PI", "E", "TAU", "INF", "NAN",
                       "INT32_MAX", "INT32_MIN", "INT64_MAX", "INT64_MIN",
                       "UINT32_MAX", "UINT64_MAX",
                       "abs", "min", "max", "clamp",
                       "sqrt", "cbrt", "pow", "hypot",
                       "exp", "exp2", "ln", "log2", "log10",
                       "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
                       "sinh", "cosh", "tanh",
                       "ceil", "floor", "round", "trunc",
                       "lerp", "map", "toRadians", "toDegrees",
                       "isNaN", "isInf", "isFinite"}},
        {"std::str", {"contains", "startsWith", "endsWith",
                      "indexOf", "lastIndexOf", "count",
                      "toUpper", "toLower",
                      "trim", "trimLeft", "trimRight",
                      "replace", "replaceFirst",
                      "repeat", "reverse",
                      "padLeft", "padRight",
                      "substring", "charAt", "slice",
                      "parseInt", "parseIntRadix", "parseFloat",
                      "fromCharCode",
                      "split", "splitN", "joinVec", "lines",
                      "chars", "fromChars", "toBytes", "fromBytes"}},
        {"std::fmt", {"lpad", "rpad", "center",
                      "hex", "hexUpper", "oct", "bin",
                      "fixed", "scientific",
                      "humanBytes", "commas", "percent"}},
        {"std::conv", {"itoa", "itoaRadix", "utoa",
                       "ftoa", "ftoaPrecision",
                       "atoi", "atof",
                       "toHex", "toOctal", "toBinary", "fromHex",
                       "charToInt", "intToChar"}},
        {"std::collections", {"Vec", "Map", "Set"}},
        {"std::mem", {"alloc", "allocZeroed", "realloc", "free",
                      "copy", "move", "set", "zero", "compare"}},
        {"std::time", {"now", "nowNanos", "nowMicros",
                       "sleep", "sleepMicros", "clock",
                       "year", "month", "day",
                       "hour", "minute", "second", "weekday",
                       "timestamp", "elapsed",
                       "formatTime", "parseTime"}},
        {"std::random", {"seed", "seedTime",
                         "randInt", "randIntRange",
                         "randUint", "randFloat", "randFloatRange",
                         "randBool", "randChar"}},
        {"std::fs", {"readFile", "writeFile", "appendFile",
                     "exists", "isFile", "isDir",
                     "remove", "removeDir", "rename",
                     "mkdir", "mkdirAll",
                     "listDir", "readBytes", "writeBytes",
                     "fileSize", "cwd", "setCwd", "tempDir"}},
        {"std::path", {"join", "parent", "fileName", "stem",
                       "extension", "isAbsolute", "isRelative",
                       "normalize", "toAbsolute", "separator",
                       "withExtension", "withFileName", "joinAll"}},
        {"std::process", {"exit", "abort",
                          "env", "setEnv", "hasEnv",
                          "exec", "execOutput",
                          "pid", "platform", "arch",
                          "homeDir", "executablePath"}},
        {"std::os", {"getpid", "getppid", "getuid", "getgid",
                     "hostname", "pageSize",
                     "errno", "strerror", "kill",
                     "dup", "dup2", "closeFd"}},
        {"std::net", {"tcpConnect", "tcpListen", "tcpAccept",
                      "tcpSend", "tcpRecv",
                      "udpBind", "udpSendTo", "udpRecvFrom",
                      "close", "setTimeout", "resolve"}},
        {"std::regex", {"match", "find", "findIndex",
                        "regexReplace", "regexReplaceFirst",
                        "isValid", "findAll", "regexSplit"}},
        {"std::encoding", {"base64EncodeStr", "base64DecodeStr",
                           "urlEncode", "urlDecode",
                           "base64Encode", "base64Decode"}},
        {"std::crypto", {"md5String", "sha1String",
                         "sha256String", "sha512String",
                         "md5", "sha1", "sha256", "sha512",
                         "hmacSha256", "randomBytes"}},
        {"std::compress", {"gzipCompress", "gzipDecompress",
                           "deflate", "inflate", "compressLevel"}},
        {"std::hash", {"hashString", "hashInt", "hashCombine",
                       "hashBytes", "crc32"}},
        {"std::bits", {"popcount", "ctz", "clz",
                       "rotl", "rotr", "bswap",
                       "isPow2", "nextPow2", "bitReverse",
                       "extractBits", "setBit", "clearBit",
                       "toggleBit", "testBit"}},
        {"std::ascii", {"isAlpha", "isDigit", "isAlphaNum",
                        "isUpper", "isLower", "isWhitespace",
                        "isPrintable", "isControl", "isHexDigit",
                        "isAscii",
                        "toUpper", "toLower",
                        "toDigit", "fromDigit"}},
        {"std::thread", {"cpuCount", "threadId", "yield", "Task", "Mutex"}},
        {"std::test", {"assertEqual", "assertNotEqual",
                       "assertTrue", "assertFalse",
                       "assertGreater", "assertLess",
                       "assertGreaterEq", "assertLessEq",
                       "assertStringContains", "assertNear",
                       "fail", "skip", "log"}},
    };

    // Collect the set of std submodule names
    static const std::vector<std::string> stdSubmodules = [] {
        std::vector<std::string> result;
        for (auto& [fullPath, _] : stdModules) {
            // "std::log" → "log"
            auto pos = fullPath.find("::");
            if (pos != std::string::npos)
                result.push_back(fullPath.substr(pos + 2));
        }
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }();

    // Case 1: modulePath is empty — completing right after "use "
    // Suggest "std" + project namespaces
    if (modulePath.empty()) {
        if (matchesPrefix("std", prefix)) {
            CompletionItem ci;
            ci.label = "std";
            ci.kind = CompletionKind::Module;
            ci.detail = "standard library";
            ci.insertText = "std::";
            items.push_back(std::move(ci));
        }
        // Project namespaces
        if (project && project->isValid()) {
            for (auto& ns : project->registry().allNamespaces()) {
                // Skip "std::" prefixed
                if (ns.substr(0, 5) == "std::") continue;
                // For "MyNamespace" — offer it
                auto topLevel = ns;
                auto sep = ns.find("::");
                if (sep != std::string::npos)
                    topLevel = ns.substr(0, sep);
                if (!matchesPrefix(topLevel, prefix)) continue;
                CompletionItem ci;
                ci.label = topLevel;
                ci.kind = CompletionKind::Module;
                ci.detail = "namespace";
                ci.insertText = topLevel + "::";
                items.push_back(std::move(ci));
            }
        }
        return;
    }

    // Case 2: modulePath is "std" — completing after "use std::"
    // Suggest submodule names (log, math, collections, etc.)
    if (modulePath == "std") {
        for (auto& sub : stdSubmodules) {
            if (!matchesPrefix(sub, prefix)) continue;
            CompletionItem ci;
            ci.label = sub;
            ci.kind = CompletionKind::Module;
            ci.detail = "std::" + sub;
            ci.insertText = sub + "::";
            items.push_back(std::move(ci));
        }
        return;
    }

    // Case 3: modulePath is a full std module (e.g. "std::log") — suggest symbols
    auto it = stdModules.find(modulePath);
    if (it != stdModules.end()) {
        for (auto& sym : it->second) {
            if (!matchesPrefix(sym, prefix)) continue;
            CompletionItem ci;
            ci.label = sym;
            // Heuristic: uppercase = type/constant, lowercase = function
            if (!sym.empty() && std::isupper(sym[0])) {
                ci.kind = CompletionKind::Class;
                ci.detail = "type";
            } else {
                ci.kind = CompletionKind::Function;
                ci.detail = "function";
            }
            items.push_back(std::move(ci));
        }
        return;
    }

    // Case 4: User namespace — suggest exported symbols
    if (project && project->isValid()) {
        auto syms = project->registry().getNamespaceSymbols(modulePath);
        for (auto* sym : syms) {
            if (!matchesPrefix(sym->name, prefix)) continue;
            CompletionItem ci;
            ci.label = sym->name;
            switch (sym->kind) {
                case ExportedSymbol::Function:
                    ci.kind = CompletionKind::Function;
                    ci.detail = "function";
                    break;
                case ExportedSymbol::Struct:
                    ci.kind = CompletionKind::Struct;
                    ci.detail = "struct";
                    break;
                case ExportedSymbol::Enum:
                    ci.kind = CompletionKind::Enum;
                    ci.detail = "enum";
                    break;
                case ExportedSymbol::Union:
                    ci.kind = CompletionKind::Struct;
                    ci.detail = "union";
                    break;
                case ExportedSymbol::TypeAlias:
                    ci.kind = CompletionKind::Class;
                    ci.detail = "type alias";
                    break;
                case ExportedSymbol::ExtendBlock:
                    continue;
            }
            items.push_back(std::move(ci));
        }
    }
}

void CompletionProvider::addEnumVariants(std::vector<CompletionItem>& items,
                                         const std::string& enumName,
                                         LuxParser::ProgramContext* tree,
                                         const CBindings& bindings,
                                         const ProjectContext* project) {
    // Same-file enum
    auto* ed = findEnumDecl(tree, enumName);
    if (ed) {
        auto ids = ed->IDENTIFIER();
        for (size_t i = 1; i < ids.size(); i++) {
            CompletionItem ci;
            ci.label = ids[i]->getText();
            ci.kind = CompletionKind::EnumMember;
            ci.detail = enumName + "::" + ids[i]->getText();
            items.push_back(std::move(ci));
        }
        return;
    }

    // Cross-file enum
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto* sym = project->registry().findSymbol(ns, enumName);
            if (!sym || sym->kind != ExportedSymbol::Enum) continue;
            auto* decl = dynamic_cast<LuxParser::EnumDeclContext*>(sym->decl);
            if (!decl) continue;
            auto ids = decl->IDENTIFIER();
            for (size_t i = 1; i < ids.size(); i++) {
                CompletionItem ci;
                ci.label = ids[i]->getText();
                ci.kind = CompletionKind::EnumMember;
                ci.detail = enumName + "::" + ids[i]->getText();
                items.push_back(std::move(ci));
            }
            return;
        }
    }

    // C enum
    auto* ce = bindings.findEnum(enumName);
    if (ce) {
        for (auto& [valName, val] : ce->values) {
            CompletionItem ci;
            ci.label = valName;
            ci.kind = CompletionKind::EnumMember;
            ci.detail = enumName + "::" + valName + " = " + std::to_string(val);
            items.push_back(std::move(ci));
        }
    }
}

void CompletionProvider::addStaticMethods(std::vector<CompletionItem>& items,
                                          const std::string& typeName,
                                          LuxParser::ProgramContext* tree,
                                          const ProjectContext* project) {
    // Same-file extend blocks
    for (auto* tld : tree->topLevelDecl()) {
        auto* ext = tld->extendDecl();
        if (!ext || ext->IDENTIFIER()->getText() != typeName) continue;
        for (auto* m : ext->extendMethod()) {
            bool isStatic = (m->AMPERSAND() == nullptr);
            if (!isStatic) continue;
            CompletionItem ci;
            std::string mName = m->IDENTIFIER(0)->getText();
            ci.label = mName;
            ci.kind = CompletionKind::Function;
            ci.detail = formatMethodSignature(m);
            ci.documentation = "```lux\n" + ci.detail + "\n```";
            if (auto* pl = m->paramList(); pl && !pl->param().empty()) {
                auto pList = pl->param();
                std::string snippet = mName + "(";
                for (size_t i = 0; i < pList.size(); i++) {
                    if (i > 0) snippet += ", ";
                    snippet += "${" + std::to_string(i + 1) + ":" +
                               pList[i]->IDENTIFIER()->getText() + "}";
                }
                snippet += ")";
                ci.insertText = snippet;
                ci.insertTextFormat = InsertTextFormat::Snippet;
            } else {
                ci.insertText = mName + "()";
            }
            items.push_back(std::move(ci));
        }
    }

    // Cross-file extend blocks
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto syms = project->registry().getNamespaceSymbols(ns);
            for (auto* sym : syms) {
                if (sym->kind != ExportedSymbol::ExtendBlock) continue;
                auto* ext = dynamic_cast<LuxParser::ExtendDeclContext*>(sym->decl);
                if (!ext || ext->IDENTIFIER()->getText() != typeName) continue;
                for (auto* m : ext->extendMethod()) {
                    bool isStatic = (m->AMPERSAND() == nullptr);
                    if (!isStatic) continue;
                    CompletionItem ci;
                    std::string mName = m->IDENTIFIER(0)->getText();
                    ci.label = mName;
                    ci.kind = CompletionKind::Function;
                    ci.detail = formatMethodSignature(m);
                    ci.documentation = "```lux\n" + ci.detail + "\n```";
                    if (auto* pl = m->paramList(); pl && !pl->param().empty()) {
                        auto pList = pl->param();
                        std::string snippet = mName + "(";
                        for (size_t i = 0; i < pList.size(); i++) {
                            if (i > 0) snippet += ", ";
                            snippet += "${" + std::to_string(i + 1) + ":" +
                                       pList[i]->IDENTIFIER()->getText() + "}";
                        }
                        snippet += ")";
                        ci.insertText = snippet;
                        ci.insertTextFormat = InsertTextFormat::Snippet;
                    } else {
                        ci.insertText = mName + "()";
                    }
                    items.push_back(std::move(ci));
                }
            }
        }
    }
}

void CompletionProvider::addTypeNames(std::vector<CompletionItem>& items,
                                      LuxParser::ProgramContext* tree,
                                      const CBindings& bindings,
                                      const ProjectContext* project,
                                      const std::string& prefix) {
    // Primitive types
    static const char* primitives[] = {
        "int8", "int16", "int32", "int64", "int128", "isize",
        "uint8", "uint16", "uint32", "uint64", "uint128", "usize",
        "float32", "float64", "float80", "float128", "double",
        "bool", "char", "void", "string", "cstring",
    };
    for (auto* p : primitives) {
        if (!matchesPrefix(p, prefix)) continue;
        CompletionItem ci;
        ci.label = p;
        ci.kind = CompletionKind::Keyword;
        ci.detail = "primitive type";
        items.push_back(std::move(ci));
    }

    // Same-file structs, enums, unions, type aliases
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* sd = tld->structDecl()) {
            std::string name = sd->IDENTIFIER()->getText();
            if (!matchesPrefix(name, prefix)) continue;
            items.push_back({name, CompletionKind::Struct, "struct " + name});
        }
        if (auto* ed = tld->enumDecl()) {
            auto ids = ed->IDENTIFIER();
            if (ids.empty()) continue;
            std::string name = ids[0]->getText();
            if (!matchesPrefix(name, prefix)) continue;
            items.push_back({name, CompletionKind::Enum, "enum " + name});
        }
        if (auto* ud = tld->unionDecl()) {
            std::string name = ud->IDENTIFIER()->getText();
            if (!matchesPrefix(name, prefix)) continue;
            items.push_back({name, CompletionKind::Struct, "union " + name});
        }
        if (auto* ta = tld->typeAliasDecl()) {
            std::string name = ta->IDENTIFIER()->getText();
            if (!matchesPrefix(name, prefix)) continue;
            items.push_back({name, CompletionKind::Class, "type " + name});
        }
    }

    // C structs/enums/typedefs
    for (auto& [name, _] : bindings.structs()) {
        if (!matchesPrefix(name, prefix)) continue;
        items.push_back({name, CompletionKind::Struct, "(C) struct " + name});
    }
    for (auto& [name, _] : bindings.enums()) {
        if (!matchesPrefix(name, prefix)) continue;
        items.push_back({name, CompletionKind::Enum, "(C) enum " + name});
    }
    for (auto& [name, _] : bindings.typedefs()) {
        if (!matchesPrefix(name, prefix)) continue;
        items.push_back({name, CompletionKind::Class, "(C) typedef " + name});
    }

    // Cross-file types
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto syms = project->registry().getNamespaceSymbols(ns);
            for (auto* sym : syms) {
                if (sym->kind != ExportedSymbol::Struct &&
                    sym->kind != ExportedSymbol::Enum &&
                    sym->kind != ExportedSymbol::Union &&
                    sym->kind != ExportedSymbol::TypeAlias) continue;
                if (!matchesPrefix(sym->name, prefix)) continue;
                CompletionKind k = CompletionKind::Class;
                if (sym->kind == ExportedSymbol::Struct) k = CompletionKind::Struct;
                else if (sym->kind == ExportedSymbol::Enum) k = CompletionKind::Enum;
                items.push_back({sym->name, k, "[" + ns + "] " + sym->name});
            }
        }
    }
}

void CompletionProvider::addKeywords(std::vector<CompletionItem>& items,
                                     const std::string& prefix) {
    static const char* keywords[] = {
        "if", "else", "for", "while", "do", "loop",
        "switch", "case", "default", "break", "continue",
        "ret", "fn", "struct", "enum", "union", "extend",
        "type", "extern", "use", "namespace",
        "try", "catch", "finally", "throw",
        "spawn", "await", "lock", "defer",
        "as", "is", "in", "sizeof", "typeof",
        "true", "false", "null",
    };

    for (auto* kw : keywords) {
        if (!matchesPrefix(kw, prefix)) continue;
        CompletionItem ci;
        ci.label = kw;
        ci.kind = CompletionKind::Keyword;
        items.push_back(std::move(ci));
    }

    // #include directive (matches on "include" or "#include")
    bool matchHash = !prefix.empty() && prefix[0] == '#'
                     && matchesPrefix("include", prefix.substr(1));
    if (matchHash || matchesPrefix("include", prefix)) {
        CompletionItem ci;
        ci.label = "#include";
        ci.kind = CompletionKind::Snippet;
        ci.detail = "C header include directive";
        ci.insertText = "#include <${1:header}>";
        ci.insertTextFormat = InsertTextFormat::Snippet;
        ci.filterText = "#include";
        items.push_back(std::move(ci));
    }
}

void CompletionProvider::addGlobalBuiltins(std::vector<CompletionItem>& items,
                                            const std::string& prefix) {
    // Global builtins always available without import
    static const struct {
        const char* name;
        const char* params;     // display params
        const char* retType;
        const char* snippet;    // nullptr = name + "()"
        const char* doc;
    } globals[] = {
        {"assert",       "(bool condition)",           "void",    "assert(${1:condition})",
         "Aborts if condition is false."},
        {"assertMsg",    "(bool condition, string msg)","void",   "assertMsg(${1:condition}, ${2:msg})",
         "Aborts with message if condition is false."},
        {"panic",        "(string msg)",               "void",    "panic(${1:msg})",
         "Immediately aborts execution with a message."},
        {"unreachable",  "()",                         "void",    nullptr,
         "Marks unreachable code. Aborts if reached."},
        {"exit",         "(int32 code)",               "void",    "exit(${1:code})",
         "Exits the process with the given status code."},
        {"toInt",        "(string s)",                 "int64",   "toInt(${1:s})",
         "Parses a string into an int64."},
        {"toFloat",      "(string s)",                 "float64", "toFloat(${1:s})",
         "Parses a string into a float64."},
        {"toBool",       "(string s)",                 "bool",    "toBool(${1:s})",
         "Parses a string into a bool."},
        {"toString",     "(T value)",                  "string",  "toString(${1:value})",
         "Converts any primitive value to string."},
        {"cstr",         "(string s)",                 "*char",   "cstr(${1:s})",
         "Converts a Lux string to a null-terminated C string."},
        {"fromCStr",     "(*char ptr)",                "string",  "fromCStr(${1:ptr})",
         "Converts a null-terminated C string to a Lux string."},
        {"fromCStrLen",  "(*char ptr, usize len)",     "string",  "fromCStrLen(${1:ptr}, ${2:len})",
         "Converts a C string with explicit length to a Lux string."},
    };

    for (auto& g : globals) {
        if (!matchesPrefix(g.name, prefix)) continue;
        CompletionItem ci;
        ci.label = g.name;
        ci.kind = CompletionKind::Function;
        ci.detail = std::string(g.params) + " -> " + g.retType;
        ci.documentation = "```lux\nfn " + std::string(g.name) +
                           g.params + " -> " + g.retType +
                           "\n```\n\n" + g.doc + "\n\n*Global builtin*";
        if (g.snippet) {
            ci.insertText = g.snippet;
            ci.insertTextFormat = InsertTextFormat::Snippet;
        } else {
            ci.insertText = std::string(g.name) + "()";
        }
        ci.sortText = "1_" + std::string(g.name); // prioritize over keywords
        items.push_back(std::move(ci));
    }
}

void CompletionProvider::addHeaderSuggestions(std::vector<CompletionItem>& items,
                                              const std::string& prefix) {
    // Cache the header list to avoid re-scanning on every keystroke
    static std::vector<std::string> cachedHeaders;
    static bool cached = false;
    if (!cached) {
        cachedHeaders = CHeaderResolver::listSystemHeaders();
        cached = true;
    }

    for (auto& h : cachedHeaders) {
        if (!matchesPrefix(h, prefix)) continue;
        CompletionItem ci;
        ci.label = h;
        ci.kind = CompletionKind::File;
        ci.detail = "C header";
        ci.insertText = h + ">";
        items.push_back(std::move(ci));
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════

std::string CompletionProvider::resolveMethodReturnType(
    const std::string& receiverType, const std::string& methodName,
    LuxParser::ProgramContext* tree, const ProjectContext* project) {

    if (receiverType.empty() || methodName.empty()) return "";

    // Helper to resolve placeholder return types
    auto resolvePlaceholder = [&](const std::string& retType) -> std::string {
        if (retType == "_self") return receiverType;
        if (retType == "_elem") {
            // Array element type
            if (receiverType.size() > 2 && receiverType[0] == '[') {
                auto cb = receiverType.find(']');
                if (cb != std::string::npos && cb + 1 < receiverType.size())
                    return receiverType.substr(cb + 1);
            }
            // Extended type element: Vec<int32> → int32
            auto ab = receiverType.find('<');
            if (ab != std::string::npos) {
                auto closeAngle = receiverType.rfind('>');
                if (closeAngle != std::string::npos && closeAngle > ab)
                    return receiverType.substr(ab + 1, closeAngle - ab - 1);
            }
            return "";
        }
        if (retType == "_key") {
            // Map<K,V> → K
            auto ab = receiverType.find('<');
            if (ab != std::string::npos) {
                auto inner = receiverType.substr(ab + 1);
                auto comma = inner.find(',');
                if (comma != std::string::npos) return inner.substr(0, comma);
            }
            return "";
        }
        if (retType == "_val") {
            // Map<K,V> → V
            auto ab = receiverType.find('<');
            if (ab != std::string::npos) {
                auto inner = receiverType.substr(ab + 1);
                auto comma = inner.find(',');
                auto closeAngle = inner.rfind('>');
                if (comma != std::string::npos && closeAngle != std::string::npos) {
                    auto val = inner.substr(comma + 1, closeAngle - comma - 1);
                    while (!val.empty() && val[0] == ' ') val.erase(0, 1);
                    return val;
                }
            }
            return "";
        }
        return retType;
    };

    // 1) Check array methods
    if (receiverType.size() > 2 && receiverType[0] == '[') {
        auto arrayMethods = methodRegistry_.arrayMethods();
        for (auto* md : arrayMethods) {
            if (md->name == methodName)
                return resolvePlaceholder(md->returnType);
        }
    }

    // 2) Check extended type methods (Vec<T>, Map<K,V>, Set<T>)
    auto angleBracket = receiverType.find('<');
    if (angleBracket != std::string::npos) {
        std::string baseName = receiverType.substr(0, angleBracket);
        auto* extDesc = extTypeRegistry_.lookup(baseName);
        if (extDesc) {
            for (auto& md : extDesc->methods) {
                if (md.name == methodName)
                    return resolvePlaceholder(md.returnType);
            }
        }
    }

    // 3) Check primitive type methods
    auto* typeInfo = typeRegistry_.lookup(receiverType);
    if (typeInfo) {
        auto methods = methodRegistry_.methodsFor(typeInfo->kind);
        for (auto* md : methods) {
            if (md->name == methodName)
                return resolvePlaceholder(md->returnType);
        }
    }

    // 4) Check extend methods defined in the same file
    if (tree) {
        for (auto* tld : tree->topLevelDecl()) {
            auto* ext = tld->extendDecl();
            if (!ext || !ext->IDENTIFIER()) continue;
            std::string extTypeName = ext->IDENTIFIER()->getText();
            if (extTypeName != receiverType) continue;

            for (auto* m : ext->extendMethod()) {
                if (!m->IDENTIFIER(0)) continue;
                if (m->IDENTIFIER(0)->getText() == methodName) {
                    if (m->typeSpec())
                        return m->typeSpec()->getText();
                    return "void";
                }
            }
        }
    }

    // 5) Check extend methods from project (cross-file)
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto syms = project->registry().getNamespaceSymbols(ns);
            for (auto* sym : syms) {
                if (sym->kind != ExportedSymbol::ExtendBlock) continue;
                auto* ext = dynamic_cast<LuxParser::ExtendDeclContext*>(sym->decl);
                if (!ext || !ext->IDENTIFIER()) continue;
                if (ext->IDENTIFIER()->getText() != receiverType) continue;

                for (auto* m : ext->extendMethod()) {
                    if (!m->IDENTIFIER(0)) continue;
                    if (m->IDENTIFIER(0)->getText() == methodName) {
                        if (m->typeSpec())
                            return m->typeSpec()->getText();
                        return "void";
                    }
                }
            }
        }
    }

    return "";
}

std::unordered_map<std::string, CompletionProvider::LocalVar>
CompletionProvider::collectLocals(LuxParser::FunctionDeclContext* func,
                                  size_t beforeLine) {
    std::unordered_map<std::string, LocalVar> result;

    if (auto* params = func->paramList()) {
        for (auto* p : params->param()) {
            result[p->IDENTIFIER()->getText()] = {p->typeSpec()->getText(), 0};
        }
    }

    collectLocalsFromBlock(func->block(), beforeLine, result);
    return result;
}

std::unordered_map<std::string, CompletionProvider::LocalVar>
CompletionProvider::collectLocalsFromMethod(LuxParser::ExtendMethodContext* method,
                                            size_t beforeLine) {
    std::unordered_map<std::string, LocalVar> result;

    bool isInstance = (method->AMPERSAND() != nullptr);
    if (isInstance) {
        // &self param
        result[method->IDENTIFIER(1)->getText()] = {"&self", 0};
        // Extra params
        for (auto* p : method->param()) {
            result[p->IDENTIFIER()->getText()] = {p->typeSpec()->getText(), 0};
        }
    } else {
        if (auto* pl = method->paramList()) {
            for (auto* p : pl->param()) {
                result[p->IDENTIFIER()->getText()] = {p->typeSpec()->getText(), 0};
            }
        }
    }

    collectLocalsFromBlock(method->block(), beforeLine, result);
    return result;
}

LuxParser::FunctionDeclContext*
CompletionProvider::findEnclosingFunction(LuxParser::ProgramContext* tree,
                                          size_t line) {
    size_t tokenLine = line + 1; // 0-based → 1-based
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* func = tld->functionDecl()) {
            auto* start = func->getStart();
            auto* stop  = func->getStop();
            if (start && stop &&
                tokenLine >= start->getLine() && tokenLine <= stop->getLine())
                return func;
        }
    }
    return nullptr;
}

LuxParser::ExtendMethodContext*
CompletionProvider::findEnclosingMethod(LuxParser::ProgramContext* tree,
                                        size_t line) {
    size_t tokenLine = line + 1;
    for (auto* tld : tree->topLevelDecl()) {
        auto* ext = tld->extendDecl();
        if (!ext) continue;
        for (auto* method : ext->extendMethod()) {
            auto* start = method->getStart();
            auto* stop  = method->getStop();
            if (start && stop &&
                tokenLine >= start->getLine() && tokenLine <= stop->getLine())
                return method;
        }
    }
    return nullptr;
}

LuxParser::StructDeclContext*
CompletionProvider::findStructDecl(LuxParser::ProgramContext* tree,
                                   const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* sd = tld->structDecl()) {
            if (sd->IDENTIFIER()->getText() == name) return sd;
        }
    }
    return nullptr;
}

LuxParser::EnumDeclContext*
CompletionProvider::findEnumDecl(LuxParser::ProgramContext* tree,
                                 const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* ed = tld->enumDecl()) {
            auto ids = ed->IDENTIFIER();
            if (!ids.empty() && ids[0]->getText() == name) return ed;
        }
    }
    return nullptr;
}

LuxParser::ExtendDeclContext*
CompletionProvider::findExtendDecl(LuxParser::ProgramContext* tree,
                                   const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* ext = tld->extendDecl()) {
            if (ext->IDENTIFIER()->getText() == name) return ext;
        }
    }
    return nullptr;
}

std::string CompletionProvider::inferVarType(const std::string& varName,
                                             LuxParser::ProgramContext* tree,
                                             size_t cursorLine) {
    // Check function locals
    auto* func = findEnclosingFunction(tree, cursorLine);
    if (func) {
        auto locals = collectLocals(func, cursorLine);
        auto it = locals.find(varName);
        if (it != locals.end()) return it->second.typeName;
    }

    // Check extend method locals
    auto* method = findEnclosingMethod(tree, cursorLine);
    if (method) {
        auto locals = collectLocalsFromMethod(method, cursorLine);
        auto it = locals.find(varName);
        if (it != locals.end()) return it->second.typeName;
    }

    return "";
}

std::string CompletionProvider::formatFuncSignature(LuxParser::FunctionDeclContext* func) {
    std::string sig = typeSpecToString(func->typeSpec()) + " " +
                      func->IDENTIFIER()->getText() + "(";
    if (auto* params = func->paramList()) {
        bool first = true;
        for (auto* p : params->param()) {
            if (!first) sig += ", ";
            first = false;
            sig += typeSpecToString(p->typeSpec());
            if (p->SPREAD()) sig += " ...";
            else sig += " ";
            sig += p->IDENTIFIER()->getText();
        }
    }
    sig += ")";
    return sig;
}

std::string CompletionProvider::formatMethodSignature(LuxParser::ExtendMethodContext* method) {
    std::string sig = typeSpecToString(method->typeSpec()) + " " +
                      method->IDENTIFIER(0)->getText() + "(";
    bool isInstance = (method->AMPERSAND() != nullptr);
    if (isInstance) {
        sig += "&" + method->IDENTIFIER(1)->getText();
        bool first = true;
        for (auto* p : method->param()) {
            sig += ", ";
            sig += typeSpecToString(p->typeSpec()) + " " + p->IDENTIFIER()->getText();
        }
    } else {
        if (auto* pl = method->paramList()) {
            bool first = true;
            for (auto* p : pl->param()) {
                if (!first) sig += ", ";
                first = false;
                sig += typeSpecToString(p->typeSpec()) + " " + p->IDENTIFIER()->getText();
            }
        }
    }
    sig += ")";
    return sig;
}

std::string CompletionProvider::typeSpecToString(LuxParser::TypeSpecContext* ts) {
    if (!ts) return "?";
    return ts->getText();
}

bool CompletionProvider::matchesPrefix(const std::string& name,
                                       const std::string& prefix) {
    if (prefix.empty()) return true;
    if (name.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); i++) {
        if (std::tolower(name[i]) != std::tolower(prefix[i])) return false;
    }
    return true;
}

void CompletionProvider::dedup(std::vector<CompletionItem>& items) {
    std::unordered_set<std::string> seen;
    auto it = std::remove_if(items.begin(), items.end(),
        [&seen](const CompletionItem& item) {
            return !seen.insert(item.label).second;
        });
    items.erase(it, items.end());
}
