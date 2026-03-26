#include "lsp/HoverProvider.h"
#include "lsp/ProjectContext.h"
#include "parser/Parser.h"
#include "ffi/CHeaderResolver.h"
#include "namespace/NamespaceRegistry.h"

#include <sstream>

// Forward declarations of static helpers used in hoverIdent
static void collectLocalsFromBlock(
        LuxParser::BlockContext* block,
        size_t beforeLine,
        std::unordered_map<std::string, HoverProvider::LocalVar>& out);

// ═══════════════════════════════════════════════════════════════════════
//  Main entry point (single-file mode)
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hover(const std::string& source,
                                                 size_t line, size_t col) {
    return hover(source, line, col, "", nullptr);
}

// ═══════════════════════════════════════════════════════════════════════
//  Main entry point (project-aware mode)
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hover(const std::string& source,
                                                 size_t line, size_t col,
                                                 const std::string& filePath,
                                                 const ProjectContext* project) {
    auto parsed = Parser::parseString(source);
    if (!parsed.tree) return std::nullopt;

    // Resolve C headers — use project-level bindings if available.
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

    // Find the token at the cursor (LSP 0-based → ANTLR 1-based line).
    size_t tokenLine = line + 1;
    antlr4::Token* hoveredToken = nullptr;

    for (auto& token : parsed.tokens->getTokens()) {
        if (token->getType() == antlr4::Token::EOF) continue;
        if (token->getLine() == tokenLine) {
            size_t start = token->getCharPositionInLine();
            size_t end   = start + token->getText().length();
            if (col >= start && col < end) {
                hoveredToken = token;
                break;
            }
        }
    }
    if (!hoveredToken) return std::nullopt;

    // Skip non-identifier tokens (operators, punctuation, keywords, etc.)
    // We do handle some keyword tokens for type names below.
    auto tokenType = hoveredToken->getType();
    std::string tokenText = hoveredToken->getText();

    // Walk the parse tree to find the context of this token.
    // Strategy: iterate top-level declarations and expressions to find a match.

    // ── Namespace declaration: namespace Main; ──────────────────────
    if (auto* nsDecl = parsed.tree->namespaceDecl()) {
        if (nsDecl->IDENTIFIER()->getSymbol() == hoveredToken) {
            std::string md = "```lux\n(namespace) " + tokenText + "\n```";
            return makeResult(hoveredToken, md);
        }
    }

    // ── Use declarations: use Utils::{ add, Role, User }; ──────────
    for (auto* useDecl : parsed.tree->useDecl()) {
        if (auto* item = dynamic_cast<LuxParser::UseItemContext*>(useDecl)) {
            // Hover on the imported symbol name
            if (item->IDENTIFIER()->getSymbol() == hoveredToken) {
                auto symbolName = item->IDENTIFIER()->getText();
                // Resolve via project context
                std::string modulePath;
                for (auto* id : item->modulePath()->IDENTIFIER()) {
                    if (!modulePath.empty()) modulePath += "::";
                    modulePath += id->getText();
                }
                return hoverImportedSymbol(symbolName, modulePath,
                    hoveredToken, parsed.tree, *cBindingsPtr, project);
            }
            // Hover on module path identifiers
            for (auto* id : item->modulePath()->IDENTIFIER()) {
                if (id->getSymbol() == hoveredToken) {
                    std::string md = "```lux\n(module) " + item->modulePath()->getText() + "\n```";
                    return makeResult(hoveredToken, md);
                }
            }
        }
        if (auto* group = dynamic_cast<LuxParser::UseGroupContext*>(useDecl)) {
            std::string modulePath;
            for (auto* id : group->modulePath()->IDENTIFIER()) {
                if (!modulePath.empty()) modulePath += "::";
                modulePath += id->getText();
            }
            // Hover on module path identifiers
            for (auto* id : group->modulePath()->IDENTIFIER()) {
                if (id->getSymbol() == hoveredToken) {
                    std::string md = "```lux\n(module) " + modulePath + "\n```";
                    return makeResult(hoveredToken, md);
                }
            }
            // Hover on imported symbol names
            for (auto* id : group->IDENTIFIER()) {
                if (id->getSymbol() == hoveredToken) {
                    return hoverImportedSymbol(id->getText(), modulePath,
                        hoveredToken, parsed.tree, *cBindingsPtr, project);
                }
            }
        }
    }

    // ── Check if hovering a function name in a functionDecl ─────────
    for (auto* tld : parsed.tree->topLevelDecl()) {
        if (auto* func = tld->functionDecl()) {
            auto* nameToken = func->IDENTIFIER()->getSymbol();
            if (nameToken == hoveredToken) {
                return HoverResult{formatFunctionDecl(func),
                                   line, col,
                                   line, col + tokenText.size()};
            }
            // Check parameter names
            if (auto* params = func->paramList()) {
                for (auto* p : params->param()) {
                    auto* pName = p->IDENTIFIER()->getSymbol();
                    if (pName == hoveredToken) {
                        std::string typeStr = typeSpecToString(p->typeSpec());
                        std::string md = "```lux\n(parameter) " + typeStr + " "
                                       + tokenText + "\n```";
                        return makeResult(hoveredToken, md);
                    }
                    // Hover on param type
                    // handled by the type-spec walk below
                }
            }
        }

        // ── Struct declaration name ─────────────────────────────────
        if (auto* sd = tld->structDecl()) {
            if (sd->IDENTIFIER()->getSymbol() == hoveredToken) {
                return makeResult(hoveredToken, formatStructDecl(sd));
            }
            // Hover on field names inside struct
            for (auto* f : sd->structField()) {
                if (f->IDENTIFIER()->getSymbol() == hoveredToken) {
                    std::string md = "```lux\n(field) " + typeSpecToString(f->typeSpec())
                                   + " " + tokenText + "\n```";
                    return makeResult(hoveredToken, md);
                }
            }
        }

        // ── Enum declaration name ───────────────────────────────────
        if (auto* ed = tld->enumDecl()) {
            auto ids = ed->IDENTIFIER();
            if (ids[0]->getSymbol() == hoveredToken) {
                return makeResult(hoveredToken, formatEnumDecl(ed));
            }
            // Hover on variant name
            for (size_t i = 1; i < ids.size(); i++) {
                if (ids[i]->getSymbol() == hoveredToken) {
                    std::string md = "```lux\n(variant) " + ids[0]->getText()
                                   + "::" + ids[i]->getText() + "\n```";
                    return makeResult(hoveredToken, md);
                }
            }
        }

        // ── Union declaration name ──────────────────────────────────
        if (auto* ud = tld->unionDecl()) {
            if (ud->IDENTIFIER()->getSymbol() == hoveredToken) {
                return makeResult(hoveredToken, formatUnionDecl(ud));
            }
            for (auto* f : ud->unionField()) {
                if (f->IDENTIFIER()->getSymbol() == hoveredToken) {
                    std::string md = "```lux\n(field) " + typeSpecToString(f->typeSpec())
                                   + " " + tokenText + "\n```";
                    return makeResult(hoveredToken, md);
                }
            }
        }

        // ── Extend declaration name ─────────────────────────────────
        if (auto* ext = tld->extendDecl()) {
            if (ext->IDENTIFIER()->getSymbol() == hoveredToken) {
                std::string md = formatExtendMethods(ext);
                return makeResult(hoveredToken, md);
            }
        }

        // ── Type alias declaration ──────────────────────────────────
        if (auto* ta = tld->typeAliasDecl()) {
            if (ta->IDENTIFIER()->getSymbol() == hoveredToken) {
                std::string md = "```lux\ntype " + tokenText + " = "
                               + typeSpecToString(ta->typeSpec()) + "\n```";
                return makeResult(hoveredToken, md);
            }
        }

        // ── Extern declaration name ─────────────────────────────────
        if (auto* ext = tld->externDecl()) {
            if (ext->IDENTIFIER()->getSymbol() == hoveredToken) {
                return makeResult(hoveredToken, formatExternDecl(ext));
            }
        }
    }

    // ── Resolve identifier expressions inside function bodies ───────
    // Walk into function bodies to find expressions containing the hovered token.
    // This handles method calls, field access, enum access, static method calls,
    // type specs in varDecl/params, and plain identifiers.

    for (auto* tld : parsed.tree->topLevelDecl()) {
        auto* func = tld->functionDecl();
        if (func) {
            auto* startTok = func->getStart();
            auto* stopTok  = func->getStop();
            if (startTok && stopTok &&
                tokenLine >= startTok->getLine() && tokenLine <= stopTok->getLine()) {
                auto result = walkTreeForHover(func->block(), hoveredToken, tokenText,
                                               parsed.tree, *cBindingsPtr, line, project);
                if (result) return result;
            }
            continue;
        }

        // Walk into extend block method bodies
        auto* ext = tld->extendDecl();
        if (ext) {
            auto* startTok = ext->getStart();
            auto* stopTok  = ext->getStop();
            if (!startTok || !stopTok) continue;
            if (tokenLine < startTok->getLine() || tokenLine > stopTok->getLine())
                continue;

            // Check extend method parameter names and type specs
            for (auto* method : ext->extendMethod()) {
                auto* mStart = method->getStart();
                auto* mStop  = method->getStop();
                if (!mStart || !mStop) continue;
                if (tokenLine < mStart->getLine() || tokenLine > mStop->getLine())
                    continue;

                // Method name
                if (method->IDENTIFIER(0)->getSymbol() == hoveredToken) {
                    std::string structName = ext->IDENTIFIER()->getText();
                    bool isStatic = (method->AMPERSAND() == nullptr);
                    std::string retType = typeSpecToString(method->typeSpec());
                    std::string md = "```lux\n(" + std::string(isStatic ? "static method" : "method")
                        + ") " + retType + " " + structName + "::" + tokenText + "(";
                    std::vector<LuxParser::ParamContext*> params;
                    if (isStatic) {
                        if (auto* pl = method->paramList())
                            params = pl->param();
                    } else {
                        params = method->param();
                    }
                    bool first = true;
                    if (!isStatic) { md += "&self"; first = false; }
                    for (auto* p : params) {
                        if (!first) md += ", ";
                        first = false;
                        md += typeSpecToString(p->typeSpec()) + " " + p->IDENTIFIER()->getText();
                    }
                    md += ")\n```";
                    return makeResult(hoveredToken, md);
                }

                // Return type spec
                auto r = hoverTypeSpec(method->typeSpec(), hoveredToken,
                                        parsed.tree, *cBindingsPtr, project);
                if (r) return r;

                // Parameter names and types
                bool isStatic = (method->AMPERSAND() == nullptr);
                std::vector<LuxParser::ParamContext*> params;
                if (isStatic) {
                    if (auto* pl = method->paramList())
                        params = pl->param();
                } else {
                    params = method->param();
                }
                for (auto* p : params) {
                    if (p->IDENTIFIER()->getSymbol() == hoveredToken) {
                        std::string md = "```lux\n(parameter) "
                            + typeSpecToString(p->typeSpec()) + " "
                            + p->IDENTIFIER()->getText() + "\n```";
                        return makeResult(hoveredToken, md);
                    }
                    r = hoverTypeSpec(p->typeSpec(), hoveredToken,
                                       parsed.tree, *cBindingsPtr, project);
                    if (r) return r;
                }

                // Walk method body
                auto result = walkTreeForHover(method->block(), hoveredToken,
                    tokenText, parsed.tree, *cBindingsPtr, line, project);
                if (result) return result;
            }
        }
    }

    // ── Fallback: resolve any IDENTIFIER (global scope or missed contexts) ──
    if (tokenType == LuxLexer::IDENTIFIER) {
        return hoverIdent(tokenText, hoveredToken, parsed.tree,
                          *cBindingsPtr, line, project);
    }

    // ── Primitive type keywords (int32, float64, bool, etc.) ────────
    // These are their own token types, not IDENTIFIER.
    const TypeInfo* primitiveType = typeRegistry_.lookup(tokenText);
    if (primitiveType) {
        std::string md = "```lux\n(type) " + primitiveType->name + "\n```";
        return makeResult(hoveredToken, md);
    }

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Hover on identifier
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hoverIdent(
        const std::string& name, antlr4::Token* token,
        LuxParser::ProgramContext* tree, const CBindings& bindings,
        size_t cursorLine, const ProjectContext* project) {

    // 1) Local variable / parameter in enclosing function
    auto* func = findEnclosingFunction(tree, cursorLine);
    if (func) {
        auto locals = collectLocals(func, cursorLine);
        auto it = locals.find(name);
        if (it != locals.end()) {
            std::string prefix = "[]";
            std::string dims;
            for (unsigned i = 0; i < it->second.arrayDims; i++)
                dims += "[]";
            std::string md = "```lux\n(variable) " + dims + it->second.typeName
                           + " " + name + "\n```";
            return makeResult(token, md);
        }
    }

    // 1.5) `self` keyword inside extend block method
    if (name == "self") {
        size_t tokenLine = cursorLine + 1; // to 1-based
        for (auto* tld : tree->topLevelDecl()) {
            auto* ext = tld->extendDecl();
            if (!ext) continue;
            auto* es = ext->getStart();
            auto* ee = ext->getStop();
            if (!es || !ee) continue;
            if (tokenLine < es->getLine() || tokenLine > ee->getLine()) continue;
            std::string structName = ext->IDENTIFIER()->getText();
            std::string md = "```lux\n(self) *" + structName + "\n```";
            return makeResult(token, md);
        }
    }

    // 1.6) Parameter or local inside extend method body
    if (!func) {
        size_t tokenLine = cursorLine + 1;
        for (auto* tld : tree->topLevelDecl()) {
            auto* ext = tld->extendDecl();
            if (!ext) continue;
            for (auto* method : ext->extendMethod()) {
                auto* ms = method->getStart();
                auto* me = method->getStop();
                if (!ms || !me) continue;
                if (tokenLine < ms->getLine() || tokenLine > me->getLine()) continue;
                // Check parameters
                bool isStatic = (method->AMPERSAND() == nullptr);
                std::vector<LuxParser::ParamContext*> params;
                if (isStatic) {
                    if (auto* pl = method->paramList())
                        params = pl->param();
                } else {
                    params = method->param();
                }
                for (auto* p : params) {
                    if (p->IDENTIFIER()->getText() == name) {
                        std::string md = "```lux\n(parameter) "
                            + typeSpecToString(p->typeSpec()) + " " + name + "\n```";
                        return makeResult(token, md);
                    }
                }
                // Collect locals from method body
                std::unordered_map<std::string, LocalVar> locals;
                collectLocalsFromBlock(method->block(), tokenLine, locals);
                auto it = locals.find(name);
                if (it != locals.end()) {
                    std::string dims;
                    for (unsigned i = 0; i < it->second.arrayDims; i++)
                        dims += "[]";
                    std::string md = "```lux\n(variable) " + dims + it->second.typeName
                                   + " " + name + "\n```";
                    return makeResult(token, md);
                }
            }
        }
    }

    // 2) User-defined function
    auto* funcDecl = findFunctionDecl(tree, name);
    if (funcDecl) {
        return makeResult(token, formatFunctionDecl(funcDecl));
    }

    // 2b) Extern function declaration
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* ext = tld->externDecl()) {
            if (ext->IDENTIFIER()->getText() == name) {
                return makeResult(token, formatExternDecl(ext));
            }
        }
    }

    // 3) Builtin function
    auto* builtin = builtinRegistry_.lookup(name);
    if (builtin) {
        return makeResult(token, formatBuiltinSignature(*builtin));
    }

    // 4) Builtin constant (PI, E, INT32_MAX, etc.)
    auto& constType = builtinRegistry_.lookupConstant(name);
    if (!constType.empty()) {
        std::string md = "```lux\n(constant) " + constType + " " + name + "\n```";
        return makeResult(token, md);
    }

    // 5) C function
    auto* cfunc = bindings.findFunction(name);
    if (cfunc) {
        return makeResult(token, formatCFunction(*cfunc));
    }

    // 6) C struct (used as type or in struct literal)
    auto* cstruct = bindings.findStruct(name);
    if (cstruct) {
        return makeResult(token, formatCStruct(name, *cstruct));
    }

    // 7) C enum
    auto* cenum = bindings.findEnum(name);
    if (cenum) {
        return makeResult(token, formatCEnum(name, *cenum));
    }

    // 8) C macro constant
    auto* cmacro = bindings.findMacro(name);
    if (cmacro) {
        std::string md = "```c\n#define " + name + " " +
                         std::to_string(cmacro->value) + "\n```";
        return makeResult(token, md);
    }

    // 9) C struct macro (e.g. RAYWHITE → Color{245,245,245,255})
    auto* csmacro = bindings.findStructMacro(name);
    if (csmacro) {
        std::string md = "```c\n#define " + name + " " + csmacro->structType + "{ ";
        for (size_t i = 0; i < csmacro->fieldValues.size(); i++) {
            if (i > 0) md += ", ";
            md += std::to_string(csmacro->fieldValues[i]);
        }
        md += " }\n```";
        return makeResult(token, md);
    }

    // 9.5) C enum variant value (e.g. FLAG_VSYNC_HINT from ConfigFlags)
    for (auto& [enumName, cenum] : bindings.enums()) {
        for (auto& [vname, val] : cenum.values) {
            if (vname == name) {
                std::string md = "```c\n(enum value) " + enumName + "::" + name
                               + " = " + std::to_string(val) + "\n```\n*C enum variant*";
                return makeResult(token, md);
            }
        }
    }

    // 10) C global variable
    auto* cglobal = bindings.findGlobal(name);
    if (cglobal) {
        std::string md = "```c\n(global) " + cglobal->type->name + " " + name + "\n```";
        return makeResult(token, md);
    }

    // 11) C typedef
    auto* ctdef = bindings.findTypedef(name);
    if (ctdef) {
        std::string md = "```c\ntypedef " + ctdef->underlying->name + " " + name + "\n```";
        return makeResult(token, md);
    }

    // 12) User-defined struct type
    auto* structDecl = findStructDecl(tree, name);
    if (structDecl) {
        return makeResult(token, formatStructDecl(structDecl));
    }

    // 13) User-defined enum type
    auto* enumDecl = findEnumDecl(tree, name);
    if (enumDecl) {
        return makeResult(token, formatEnumDecl(enumDecl));
    }

    // 14) User-defined union type
    auto* unionDecl = findUnionDecl(tree, name);
    if (unionDecl) {
        return makeResult(token, formatUnionDecl(unionDecl));
    }

    // 15) Type alias
    auto* aliasDecl = findTypeAliasDecl(tree, name);
    if (aliasDecl) {
        std::string md = "```lux\ntype " + name + " = "
                       + typeSpecToString(aliasDecl->typeSpec()) + "\n```";
        return makeResult(token, md);
    }

    // 16) Extended type base name (Vec, Map, Set, Task)
    auto* extDesc = extTypeRegistry_.lookup(name);
    if (extDesc) {
        std::ostringstream ss;
        ss << "```lux\n(type) " << name;
        if (extDesc->genericArity == 1) ss << "<T>";
        else if (extDesc->genericArity == 2) ss << "<K, V>";
        ss << "\n```\n\n**Methods:**\n";
        for (auto& m : extDesc->methods) {
            ss << "- `" << m.returnType << " " << m.name << "(";
            for (size_t i = 0; i < m.paramTypes.size(); i++) {
                if (i > 0) ss << ", ";
                ss << m.paramTypes[i];
            }
            ss << ")`\n";
        }
        return makeResult(token, ss.str());
    }

    // 17) Primitive type used as identifier (should not normally happen)
    auto* typeInfo = typeRegistry_.lookup(name);
    if (typeInfo) {
        std::string md = "```lux\n(type) " + typeInfo->name + "\n```";
        return makeResult(token, md);
    }

    // 18) Cross-file symbol from project registry (imported via `use`)
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto* sym = project->registry().findSymbol(ns, name);
            if (!sym) continue;
            switch (sym->kind) {
                case ExportedSymbol::Function:
                    if (auto* fd = dynamic_cast<LuxParser::FunctionDeclContext*>(sym->decl))
                        return makeResult(token, formatFunctionDecl(fd));
                    break;
                case ExportedSymbol::Struct:
                    if (auto* sd = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl))
                        return makeResult(token, formatStructDecl(sd));
                    break;
                case ExportedSymbol::Enum:
                    if (auto* ed = dynamic_cast<LuxParser::EnumDeclContext*>(sym->decl))
                        return makeResult(token, formatEnumDecl(ed));
                    break;
                case ExportedSymbol::Union:
                    if (auto* ud = dynamic_cast<LuxParser::UnionDeclContext*>(sym->decl))
                        return makeResult(token, formatUnionDecl(ud));
                    break;
                case ExportedSymbol::TypeAlias:
                    if (auto* ta = dynamic_cast<LuxParser::TypeAliasDeclContext*>(sym->decl)) {
                        std::string md = "```lux\ntype " + name + " = "
                                       + typeSpecToString(ta->typeSpec()) + "\n```";
                        return makeResult(token, md);
                    }
                    break;
                case ExportedSymbol::ExtendBlock:
                    if (auto* ext = dynamic_cast<LuxParser::ExtendDeclContext*>(sym->decl))
                        return makeResult(token, formatExtendMethods(ext));
                    break;
            }
        }
    }

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Hover on imported symbol in `use` declaration
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hoverImportedSymbol(
        const std::string& symbolName,
        const std::string& modulePath,
        antlr4::Token* token,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        const ProjectContext* project) {
    // Try project registry first
    if (project && project->isValid()) {
        auto* sym = project->registry().findSymbol(modulePath, symbolName);
        if (sym) {
            switch (sym->kind) {
                case ExportedSymbol::Function:
                    if (auto* fd = dynamic_cast<LuxParser::FunctionDeclContext*>(sym->decl))
                        return makeResult(token, formatFunctionDecl(fd));
                    break;
                case ExportedSymbol::Struct:
                    if (auto* sd = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl))
                        return makeResult(token, formatStructDecl(sd));
                    break;
                case ExportedSymbol::Enum:
                    if (auto* ed = dynamic_cast<LuxParser::EnumDeclContext*>(sym->decl))
                        return makeResult(token, formatEnumDecl(ed));
                    break;
                case ExportedSymbol::Union:
                    if (auto* ud = dynamic_cast<LuxParser::UnionDeclContext*>(sym->decl))
                        return makeResult(token, formatUnionDecl(ud));
                    break;
                case ExportedSymbol::TypeAlias:
                    if (auto* ta = dynamic_cast<LuxParser::TypeAliasDeclContext*>(sym->decl)) {
                        std::string md = "```lux\ntype " + symbolName + " = "
                                       + typeSpecToString(ta->typeSpec()) + "\n```";
                        return makeResult(token, md);
                    }
                    break;
                case ExportedSymbol::ExtendBlock:
                    if (auto* ext = dynamic_cast<LuxParser::ExtendDeclContext*>(sym->decl))
                        return makeResult(token, formatExtendMethods(ext));
                    break;
            }
        }
    }
    // Fallback to builtins
    auto* builtin = builtinRegistry_.lookup(symbolName);
    if (builtin)
        return makeResult(token, formatBuiltinSignature(*builtin));

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Recursive expression tree walker for hover
// ═══════════════════════════════════════════════════════════════════════

// Check if a token belongs to a parse tree node's token range.
static bool containsToken(antlr4::ParserRuleContext* ctx, antlr4::Token* token) {
    if (!ctx || !token) return false;
    auto* start = ctx->getStart();
    auto* stop  = ctx->getStop();
    if (!start || !stop) return false;
    size_t idx = token->getTokenIndex();
    return idx >= start->getTokenIndex() && idx <= stop->getTokenIndex();
}

std::optional<HoverResult> HoverProvider::walkExprForHover(
        LuxParser::ExpressionContext* expr,
        antlr4::Token* hoveredToken,
        const std::string& tokenText,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        size_t cursorLine,
        const ProjectContext* project) {
    if (!expr || !containsToken(expr, hoveredToken)) return std::nullopt;

    // ── Method call: expr.method(args) ──────────────────────────────
    if (auto* mc = dynamic_cast<LuxParser::MethodCallExprContext*>(expr)) {
        auto* methodId = mc->IDENTIFIER();
        if (methodId && methodId->getSymbol() == hoveredToken) {
            return hoverMethodCall(mc, tree, bindings, cursorLine, project);
        }
        // Recurse into receiver and args
        auto r = walkExprForHover(mc->expression(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
        if (r) return r;
        if (auto* al = mc->argList()) {
            for (auto* a : al->expression()) {
                r = walkExprForHover(a, hoveredToken, tokenText,
                                      tree, bindings, cursorLine, project);
                if (r) return r;
            }
        }
        return std::nullopt;
    }

    // ── Field access: expr.field ────────────────────────────────────
    if (auto* fa = dynamic_cast<LuxParser::FieldAccessExprContext*>(expr)) {
        auto* fieldId = fa->IDENTIFIER();
        if (fieldId && fieldId->getSymbol() == hoveredToken) {
            return hoverFieldAccess(fa, tree, bindings, cursorLine, project);
        }
        return walkExprForHover(fa->expression(), hoveredToken, tokenText,
                                 tree, bindings, cursorLine, project);
    }

    // ── Arrow access: expr->field (pointer dereference + field) ─────
    if (auto* aa = dynamic_cast<LuxParser::ArrowAccessExprContext*>(expr)) {
        auto* fieldId = aa->IDENTIFIER();
        if (fieldId && fieldId->getSymbol() == hoveredToken) {
            std::string fieldName = fieldId->getText();
            auto* receiver = aa->expression();
            auto* token = fieldId->getSymbol();

            // Resolve receiver pointer type
            std::string ptrTypeName;
            if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(receiver)) {
                std::string varName = ident->IDENTIFIER()->getText();
                auto* func = findEnclosingFunction(tree, cursorLine);
                if (func) {
                    auto locals = collectLocals(func, cursorLine);
                    auto it = locals.find(varName);
                    if (it != locals.end()) {
                        ptrTypeName = it->second.typeName;
                    }
                }
            }

            // Dereference: strip leading '*' to get struct name
            std::string structName;
            if (!ptrTypeName.empty() && ptrTypeName[0] == '*') {
                structName = ptrTypeName.substr(1);
            }

            // Search local structs
            if (!structName.empty()) {
                auto* sd = findStructDecl(tree, structName);
                if (sd) {
                    for (auto* f : sd->structField()) {
                        if (f->IDENTIFIER()->getText() == fieldName) {
                            std::string md = "```lux\n(field) "
                                + typeSpecToString(f->typeSpec()) + " "
                                + structName + "." + fieldName + "\n```";
                            return makeResult(token, md);
                        }
                    }
                }
            }

            // Search cross-file structs
            if (project && project->isValid()) {
                for (auto& ns : project->registry().allNamespaces()) {
                    auto syms = project->registry().getNamespaceSymbols(ns);
                    for (auto* sym : syms) {
                        if (sym->kind != ExportedSymbol::Struct) continue;
                        auto* sd = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl);
                        if (!sd) continue;
                        if (!structName.empty() &&
                            sd->IDENTIFIER()->getText() != structName)
                            continue;
                        for (auto* f : sd->structField()) {
                            if (f->IDENTIFIER()->getText() == fieldName) {
                                std::string md = "```lux\n(field) "
                                    + typeSpecToString(f->typeSpec()) + " "
                                    + sd->IDENTIFIER()->getText() + "."
                                    + fieldName + "\n```";
                                return makeResult(token, md);
                            }
                        }
                    }
                }
            }

            // Search C structs
            if (!structName.empty()) {
                auto* cs = bindings.findStruct(structName);
                if (cs) {
                    for (auto& f : cs->fields) {
                        if (f.name == fieldName) {
                            std::string md = "```c\n(field) " + f.typeInfo->name
                                + " " + structName + "." + fieldName
                                + "\n```\n*C struct field*";
                            return makeResult(token, md);
                        }
                    }
                }
            }

            return std::nullopt;
        }
        return walkExprForHover(aa->expression(), hoveredToken, tokenText,
                                 tree, bindings, cursorLine, project);
    }

    // ── Address-of: &identifier ─────────────────────────────────────
    if (auto* ao = dynamic_cast<LuxParser::AddrOfExprContext*>(expr)) {
        auto* id = ao->IDENTIFIER();
        if (id && id->getSymbol() == hoveredToken) {
            return hoverIdent(id->getText(), hoveredToken, tree, bindings,
                               cursorLine, project);
        }
        return std::nullopt;
    }

    // ── Dereference: *expr ──────────────────────────────────────────
    if (auto* de = dynamic_cast<LuxParser::DerefExprContext*>(expr)) {
        return walkExprForHover(de->expression(), hoveredToken, tokenText,
                                 tree, bindings, cursorLine, project);
    }

    // ── Enum access: Type::Variant ──────────────────────────────────
    if (auto* ea = dynamic_cast<LuxParser::EnumAccessExprContext*>(expr)) {
        auto ids = ea->IDENTIFIER();
        if (ids.size() >= 2) {
            // Hover on variant name
            if (ids[1]->getSymbol() == hoveredToken) {
                return hoverEnumAccess(ea, tree, bindings, project);
            }
            // Hover on enum type name
            if (ids[0]->getSymbol() == hoveredToken) {
                std::string typeName = ids[0]->getText();
                return hoverIdent(typeName, hoveredToken, tree, bindings,
                                   cursorLine, project);
            }
        }
        return std::nullopt;
    }

    // ── Static method call: Type::method(args) ─────────────────────
    if (auto* smc = dynamic_cast<LuxParser::StaticMethodCallExprContext*>(expr)) {
        auto ids = smc->IDENTIFIER();
        if (ids.size() >= 2) {
            // Hover on method name
            if (ids[1]->getSymbol() == hoveredToken) {
                return hoverStaticMethodCall(smc, tree, bindings, cursorLine, project);
            }
            // Hover on type name
            if (ids[0]->getSymbol() == hoveredToken) {
                std::string typeName = ids[0]->getText();
                return hoverIdent(typeName, hoveredToken, tree, bindings,
                                   cursorLine, project);
            }
        }
        if (auto* al = smc->argList()) {
            for (auto* a : al->expression()) {
                auto r = walkExprForHover(a, hoveredToken, tokenText,
                                           tree, bindings, cursorLine, project);
                if (r) return r;
            }
        }
        return std::nullopt;
    }

    // ── Function call: expr(args) ──────────────────────────────────
    if (auto* fc = dynamic_cast<LuxParser::FnCallExprContext*>(expr)) {
        auto r = walkExprForHover(fc->expression(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
        if (r) return r;
        if (auto* al = fc->argList()) {
            for (auto* a : al->expression()) {
                r = walkExprForHover(a, hoveredToken, tokenText,
                                      tree, bindings, cursorLine, project);
                if (r) return r;
            }
        }
        return std::nullopt;
    }

    // ── Struct literal: Type { field: expr, ... } ────────────────────
    if (auto* sl = dynamic_cast<LuxParser::StructLitExprContext*>(expr)) {
        auto ids = sl->IDENTIFIER();
        if (!ids.empty()) {
            // Hover on struct type name
            if (ids[0]->getSymbol() == hoveredToken) {
                return hoverIdent(ids[0]->getText(), hoveredToken, tree,
                                   bindings, cursorLine, project);
            }
            // Hover on field names
            std::string structName = ids[0]->getText();
            for (size_t i = 1; i < ids.size(); i++) {
                if (ids[i]->getSymbol() != hoveredToken) continue;
                std::string fieldName = ids[i]->getText();
                // Look up field type from struct declaration
                auto* sd = findStructDecl(tree, structName);
                if (sd) {
                    for (auto* f : sd->structField()) {
                        if (f->IDENTIFIER()->getText() == fieldName) {
                            std::string md = "```lux\n(field) "
                                + typeSpecToString(f->typeSpec()) + " "
                                + structName + "." + fieldName + "\n```";
                            return makeResult(hoveredToken, md);
                        }
                    }
                }
                // Cross-file struct fields
                if (project && project->isValid()) {
                    for (auto& ns : project->registry().allNamespaces()) {
                        auto syms = project->registry().getNamespaceSymbols(ns);
                        for (auto* sym : syms) {
                            if (sym->kind != ExportedSymbol::Struct ||
                                sym->name != structName) continue;
                            auto* rsd = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl);
                            if (!rsd) continue;
                            for (auto* f : rsd->structField()) {
                                if (f->IDENTIFIER()->getText() == fieldName) {
                                    std::string md = "```lux\n(field) "
                                        + typeSpecToString(f->typeSpec()) + " "
                                        + structName + "." + fieldName + "\n```";
                                    return makeResult(hoveredToken, md);
                                }
                            }
                        }
                    }
                }
                // Union fields
                auto* ud = findUnionDecl(tree, structName);
                if (ud) {
                    for (auto* f : ud->unionField()) {
                        if (f->IDENTIFIER()->getText() == fieldName) {
                            std::string md = "```lux\n(field) "
                                + typeSpecToString(f->typeSpec()) + " "
                                + structName + "." + fieldName + "\n```";
                            return makeResult(hoveredToken, md);
                        }
                    }
                }
                // C struct fields
                if (auto* cs = bindings.findStruct(structName)) {
                    for (auto& f : cs->fields) {
                        if (f.name == fieldName) {
                            std::string md = "```lux\n(field) "
                                + f.typeInfo->name + " " + structName + "."
                                + fieldName + "\n```";
                            return makeResult(hoveredToken, md);
                        }
                    }
                }
                // Built-in structs from TypeRegistry (e.g. Error)
                auto* bti = typeRegistry_.lookup(structName);
                if (bti && bti->kind == TypeKind::Struct) {
                    for (auto& f : bti->fields) {
                        if (f.name == fieldName) {
                            std::string md = "```lux\n(field) "
                                + f.typeInfo->name + " " + structName + "."
                                + fieldName + "\n```";
                            return makeResult(hoveredToken, md);
                        }
                    }
                }
            }
        }
        // Recurse into initializer expressions
        for (auto* child : sl->expression()) {
            auto r = walkExprForHover(child, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
        return std::nullopt;
    }

    // ── Cast: expr as Type ──────────────────────────────────────────
    if (auto* cast = dynamic_cast<LuxParser::CastExprContext*>(expr)) {
        if (auto* ts = cast->typeSpec()) {
            auto r = hoverTypeSpec(ts, hoveredToken, tree, bindings, project);
            if (r) return r;
        }
        return walkExprForHover(cast->expression(), hoveredToken, tokenText,
                                 tree, bindings, cursorLine, project);
    }

    // ── Sizeof: sizeof(Type) ────────────────────────────────────────
    if (auto* sz = dynamic_cast<LuxParser::SizeofExprContext*>(expr)) {
        if (sz->SIZEOF() && sz->SIZEOF()->getSymbol() == hoveredToken) {
            return makeResult(hoveredToken,
                "```lux\n(keyword) int64 sizeof(type)\n```\n"
                "Returns the size in bytes of the given type.");
        }
        if (auto* ts = sz->typeSpec()) {
            auto r = hoverTypeSpec(ts, hoveredToken, tree, bindings, project);
            if (r) return r;
        }
        return std::nullopt;
    }

    // ── Typeof: typeof(expr) ────────────────────────────────────────
    if (auto* to = dynamic_cast<LuxParser::TypeofExprContext*>(expr)) {
        if (to->TYPEOF() && to->TYPEOF()->getSymbol() == hoveredToken) {
            return makeResult(hoveredToken,
                "```lux\n(keyword) string typeof(expression)\n```\n"
                "Returns the type name of the given expression as a string.");
        }
        return walkExprForHover(to->expression(), hoveredToken, tokenText,
                                 tree, bindings, cursorLine, project);
    }

    // ── List comprehension: [expr | for Type x in range if cond] ────
    if (auto* lc = dynamic_cast<LuxParser::ListCompExprContext*>(expr)) {
        // Hover on iterator type spec
        if (auto* ts = lc->typeSpec()) {
            auto r = hoverTypeSpec(ts, hoveredToken, tree, bindings, project);
            if (r) return r;
        }
        // Hover on iterator variable name
        if (auto* id = lc->IDENTIFIER()) {
            if (id->getSymbol() == hoveredToken) {
                std::string typeName = typeSpecToString(lc->typeSpec());
                std::string md = "```lux\n(variable) " + typeName + " "
                               + id->getText() + "\n```\n*List comprehension iterator*";
                return makeResult(hoveredToken, md);
            }
        }
        // Hover on 'for' keyword
        if (lc->FOR() && lc->FOR()->getSymbol() == hoveredToken) {
            return makeResult(hoveredToken,
                "```lux\n(keyword) for\n```\n"
                "List comprehension iterator clause.");
        }
        // Recurse into all sub-expressions (transform, range, filter)
        for (auto* e : lc->expression()) {
            auto r = walkExprForHover(e, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
        return std::nullopt;
    }

    // ── Index access: expr[expr] ────────────────────────────────────
    if (auto* idx = dynamic_cast<LuxParser::IndexExprContext*>(expr)) {
        auto exprs = idx->expression();
        // Recurse into base expression (exprs[0]) and index (exprs[1])
        for (auto* e : exprs) {
            auto r = walkExprForHover(e, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
        return std::nullopt;
    }

    // ── Array literal: [expr, expr, ...] ────────────────────────────
    if (auto* al = dynamic_cast<LuxParser::ArrayLitExprContext*>(expr)) {
        for (auto* e : al->expression()) {
            auto r = walkExprForHover(e, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
        return std::nullopt;
    }

    // ── Generic: recurse into children ─────────────────────────────
    for (auto* child : expr->children) {
        if (auto* childExpr = dynamic_cast<LuxParser::ExpressionContext*>(child)) {
            auto r = walkExprForHover(childExpr, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    return std::nullopt;
}

std::optional<HoverResult> HoverProvider::walkStmtForHover(
        LuxParser::StatementContext* stmt,
        antlr4::Token* hoveredToken,
        const std::string& tokenText,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        size_t cursorLine,
        const ProjectContext* project) {
    if (!stmt || !containsToken(stmt, hoveredToken)) return std::nullopt;

    // Check typeSpec in variable declarations
    if (auto* vd = stmt->varDeclStmt()) {
        if (auto* ts = vd->typeSpec()) {
            auto r = hoverTypeSpec(ts, hoveredToken, tree, bindings, project);
            if (r) return r;
        }
        // Check the initializer expression
        if (auto* initExpr = vd->expression()) {
            auto r = walkExprForHover(initExpr, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    // Check expressions in expression statements
    if (auto* es = stmt->exprStmt()) {
        auto r = walkExprForHover(es->expression(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
        if (r) return r;
    }

    // Check expressions in return statements
    if (auto* rs = stmt->returnStmt()) {
        if (auto* e = rs->expression()) {
            auto r = walkExprForHover(e, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    // Check assignment statements
    if (auto* as = stmt->assignStmt()) {
        for (auto* e : as->expression()) {
            auto r = walkExprForHover(e, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    // Check call statements (standalone function calls)
    if (auto* cs = stmt->callStmt()) {
        if (auto* al = cs->argList()) {
            for (auto* a : al->expression()) {
                auto r = walkExprForHover(a, hoveredToken, tokenText,
                                           tree, bindings, cursorLine, project);
                if (r) return r;
            }
        }
    }

    // Recurse into nested blocks
    if (auto* ifS = stmt->ifStmt()) {
        if (auto* cond = ifS->expression()) {
            auto r = walkExprForHover(cond, hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
        auto r = walkBlockForHover(ifS->block(), hoveredToken, tokenText,
                                    tree, bindings, cursorLine, project);
        if (r) return r;
        for (auto* elif : ifS->elseIfClause()) {
            if (auto* ec = elif->expression()) {
                r = walkExprForHover(ec, hoveredToken, tokenText,
                                      tree, bindings, cursorLine, project);
                if (r) return r;
            }
            r = walkBlockForHover(elif->block(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
            if (r) return r;
        }
        if (auto* el = ifS->elseClause()) {
            r = walkBlockForHover(el->block(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    if (auto* fs = stmt->forStmt()) {
        if (auto* fin = dynamic_cast<LuxParser::ForInStmtContext*>(fs)) {
            if (auto* ts = fin->typeSpec()) {
                auto r = hoverTypeSpec(ts, hoveredToken, tree, bindings, project);
                if (r) return r;
            }
            auto r = walkExprForHover(fin->expression(), hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
            r = walkBlockForHover(fin->block(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
            if (r) return r;
        }
        if (auto* fc = dynamic_cast<LuxParser::ForClassicStmtContext*>(fs)) {
            if (auto* ts = fc->typeSpec()) {
                auto r = hoverTypeSpec(ts, hoveredToken, tree, bindings, project);
                if (r) return r;
            }
            for (auto* e : fc->expression()) {
                auto r = walkExprForHover(e, hoveredToken, tokenText,
                                           tree, bindings, cursorLine, project);
                if (r) return r;
            }
            auto r = walkBlockForHover(fc->block(), hoveredToken, tokenText,
                                        tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    if (auto* ws = stmt->whileStmt()) {
        auto r = walkExprForHover(ws->expression(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
        if (r) return r;
        r = walkBlockForHover(ws->block(), hoveredToken, tokenText,
                               tree, bindings, cursorLine, project);
        if (r) return r;
    }

    if (auto* dw = stmt->doWhileStmt()) {
        auto r = walkBlockForHover(dw->block(), hoveredToken, tokenText,
                                    tree, bindings, cursorLine, project);
        if (r) return r;
        r = walkExprForHover(dw->expression(), hoveredToken, tokenText,
                              tree, bindings, cursorLine, project);
        if (r) return r;
    }

    if (auto* ls = stmt->loopStmt()) {
        auto r = walkBlockForHover(ls->block(), hoveredToken, tokenText,
                                    tree, bindings, cursorLine, project);
        if (r) return r;
    }

    if (auto* sw = stmt->switchStmt()) {
        auto r = walkExprForHover(sw->expression(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
        if (r) return r;
        for (auto* c : sw->caseClause()) {
            for (auto* e : c->expression()) {
                r = walkExprForHover(e, hoveredToken, tokenText,
                                      tree, bindings, cursorLine, project);
                if (r) return r;
            }
            r = walkBlockForHover(c->block(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
            if (r) return r;
        }
        if (auto* dc = sw->defaultClause()) {
            r = walkBlockForHover(dc->block(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    if (auto* tc = stmt->tryCatchStmt()) {
        auto r = walkBlockForHover(tc->block(), hoveredToken, tokenText,
                                    tree, bindings, cursorLine, project);
        if (r) return r;
        for (auto* cc : tc->catchClause()) {
            if (auto* ts = cc->typeSpec()) {
                r = hoverTypeSpec(ts, hoveredToken, tree, bindings, project);
                if (r) return r;
            }
            // Hover on the catch variable name: catch (Error e)
            if (cc->IDENTIFIER() &&
                cc->IDENTIFIER()->getSymbol() == hoveredToken) {
                std::string typeName = cc->typeSpec()
                                        ? cc->typeSpec()->getText() : "Error";
                std::string varName = cc->IDENTIFIER()->getText();
                std::string md = "```lux\n(catch) " + typeName
                               + " " + varName + "\n```";
                return makeResult(cc->IDENTIFIER()->getSymbol(), md);
            }
            r = walkBlockForHover(cc->block(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
            if (r) return r;
        }
        if (auto* fc = tc->finallyClause()) {
            r = walkBlockForHover(fc->block(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    // Throw statement — walk into the expression
    if (auto* ts = stmt->throwStmt()) {
        auto r = walkExprForHover(ts->expression(), hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
        if (r) return r;
    }

    // Defer statement — walk into the inner call/expression
    if (auto* ds = stmt->deferStmt()) {
        if (auto* cs = ds->callStmt()) {
            // Hover on the function name in defer call
            if (cs->IDENTIFIER()->getSymbol() == hoveredToken) {
                return hoverIdent(tokenText, hoveredToken, tree,
                                   bindings, cursorLine, project);
            }
            if (auto* al = cs->argList()) {
                for (auto* a : al->expression()) {
                    auto r = walkExprForHover(a, hoveredToken, tokenText,
                                               tree, bindings, cursorLine, project);
                    if (r) return r;
                }
            }
        }
        if (auto* es = ds->exprStmt()) {
            auto r = walkExprForHover(es->expression(), hoveredToken, tokenText,
                                       tree, bindings, cursorLine, project);
            if (r) return r;
        }
    }

    return std::nullopt;
}

std::optional<HoverResult> HoverProvider::walkBlockForHover(
        LuxParser::BlockContext* block,
        antlr4::Token* hoveredToken,
        const std::string& tokenText,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        size_t cursorLine,
        const ProjectContext* project) {
    if (!block) return std::nullopt;
    for (auto* stmt : block->statement()) {
        auto r = walkStmtForHover(stmt, hoveredToken, tokenText,
                                   tree, bindings, cursorLine, project);
        if (r) return r;
    }
    return std::nullopt;
}

std::optional<HoverResult> HoverProvider::walkTreeForHover(
        LuxParser::BlockContext* block,
        antlr4::Token* hoveredToken,
        const std::string& tokenText,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        size_t cursorLine,
        const ProjectContext* project) {
    return walkBlockForHover(block, hoveredToken, tokenText,
                              tree, bindings, cursorLine, project);
}

// ═══════════════════════════════════════════════════════════════════════
//  Hover on type specifier
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hoverTypeSpec(
        LuxParser::TypeSpecContext* ts,
        antlr4::Token* hoveredToken,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        const ProjectContext* project) {
    if (!ts || !containsToken(ts, hoveredToken)) return std::nullopt;

    // Check if hovering on an IDENTIFIER in the type spec (user/C type name)
    if (auto* id = ts->IDENTIFIER()) {
        if (id->getSymbol() == hoveredToken) {
            std::string name = id->getText();
            return hoverTypeName(name, hoveredToken, tree, bindings, project);
        }
    }

    // Check if hovering on a primitive type keyword
    if (auto* pt = ts->primitiveType()) {
        if (containsToken(pt, hoveredToken)) {
            std::string name = pt->getText();
            auto* ti = typeRegistry_.lookup(name);
            if (ti) {
                std::string md = "```lux\n(type) " + ti->name + "\n```";
                return makeResult(hoveredToken, md);
            }
        }
    }

    // Recurse into nested type specs (pointers, arrays, generics)
    for (auto* nested : ts->typeSpec()) {
        auto r = hoverTypeSpec(nested, hoveredToken, tree, bindings, project);
        if (r) return r;
    }

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Hover on type name (IDENTIFIER in type position)
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hoverTypeName(
        const std::string& name,
        antlr4::Token* token,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        const ProjectContext* project) {

    // User struct
    auto* sd = findStructDecl(tree, name);
    if (sd) return makeResult(token, formatStructDecl(sd));

    // User enum
    auto* ed = findEnumDecl(tree, name);
    if (ed) return makeResult(token, formatEnumDecl(ed));

    // User union
    auto* ud = findUnionDecl(tree, name);
    if (ud) return makeResult(token, formatUnionDecl(ud));

    // Type alias
    auto* ta = findTypeAliasDecl(tree, name);
    if (ta) {
        std::string md = "```lux\ntype " + name + " = "
                       + typeSpecToString(ta->typeSpec()) + "\n```";
        return makeResult(token, md);
    }

    // C struct
    auto* cs = bindings.findStruct(name);
    if (cs) return makeResult(token, formatCStruct(name, *cs));

    // C enum
    auto* ce = bindings.findEnum(name);
    if (ce) return makeResult(token, formatCEnum(name, *ce));

    // C typedef
    auto* ct = bindings.findTypedef(name);
    if (ct) {
        std::string md = "```c\ntypedef " + ct->underlying->name + " " + name + "\n```";
        return makeResult(token, md);
    }

    // Extended type (Vec, Map, Set, Task)
    auto* ext = extTypeRegistry_.lookup(name);
    if (ext) {
        std::ostringstream ss;
        ss << "```lux\n(type) " << name;
        if (ext->genericArity == 1) ss << "<T>";
        else if (ext->genericArity == 2) ss << "<K, V>";
        ss << "\n```\n\n**Methods:**\n";
        for (auto& m : ext->methods) {
            ss << "- `" << m.returnType << " " << m.name << "(";
            for (size_t i = 0; i < m.paramTypes.size(); i++) {
                if (i > 0) ss << ", ";
                ss << m.paramTypes[i];
            }
            ss << ")`\n";
        }
        return makeResult(token, ss.str());
    }

    // Built-in type (primitives or built-in structs like Error)
    auto* ti = typeRegistry_.lookup(name);
    if (ti) {
        if (ti->kind == TypeKind::Struct && !ti->fields.empty()) {
            std::ostringstream ss;
            ss << "```lux\nstruct " << ti->name << " {\n";
            for (auto& f : ti->fields) {
                ss << "    " << f.typeInfo->name << " " << f.name;
                if (f.autoFill) ss << "  // auto-filled";
                ss << "\n";
            }
            ss << "}\n```";
            return makeResult(token, ss.str());
        }
        std::string md = "```lux\n(type) " + ti->name + "\n```";
        return makeResult(token, md);
    }

    // Cross-file type from project registry
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto* sym = project->registry().findSymbol(ns, name);
            if (!sym) continue;
            if (sym->kind == ExportedSymbol::Struct) {
                if (auto* sdc = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl))
                    return makeResult(token, formatStructDecl(sdc));
            } else if (sym->kind == ExportedSymbol::Enum) {
                if (auto* edc = dynamic_cast<LuxParser::EnumDeclContext*>(sym->decl))
                    return makeResult(token, formatEnumDecl(edc));
            } else if (sym->kind == ExportedSymbol::Union) {
                if (auto* udc = dynamic_cast<LuxParser::UnionDeclContext*>(sym->decl))
                    return makeResult(token, formatUnionDecl(udc));
            } else if (sym->kind == ExportedSymbol::TypeAlias) {
                if (auto* tac = dynamic_cast<LuxParser::TypeAliasDeclContext*>(sym->decl)) {
                    std::string md = "```lux\ntype " + name + " = "
                                   + typeSpecToString(tac->typeSpec()) + "\n```";
                    return makeResult(token, md);
                }
            }
        }
    }

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Hover on method call: expr.method(args)
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hoverMethodCall(
        LuxParser::MethodCallExprContext* ctx,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        size_t cursorLine,
        const ProjectContext* project) {
    std::string methodName = ctx->IDENTIFIER()->getText();
    auto* receiver = ctx->expression();
    std::string receiverText;

    // Try to determine receiver type name from the expression
    if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(receiver)) {
        receiverText = ident->IDENTIFIER()->getText();
    }

    // 1) Check built-in type methods (int.abs, string.len, etc.)
    //    Try each primitive type kind
    for (auto kind : {TypeKind::Integer, TypeKind::Float, TypeKind::String,
                      TypeKind::Bool, TypeKind::Char}) {
        auto* md = methodRegistry_.lookup(kind, methodName);
        if (md) {
            std::ostringstream ss;
            ss << "```lux\n(method) " << md->returnType << " " << methodName << "(";
            for (size_t i = 0; i < md->paramTypes.size(); i++) {
                if (i > 0) ss << ", ";
                ss << md->paramTypes[i];
            }
            ss << ")\n```";
            return makeResult(ctx->IDENTIFIER()->getSymbol(), ss.str());
        }
    }

    // 2) Check array methods
    auto* arrayMd = methodRegistry_.lookupArrayMethod(methodName);
    if (arrayMd) {
        std::ostringstream ss;
        ss << "```lux\n(method) " << arrayMd->returnType << " " << methodName << "(";
        for (size_t i = 0; i < arrayMd->paramTypes.size(); i++) {
            if (i > 0) ss << ", ";
            ss << arrayMd->paramTypes[i];
        }
        ss << ")\n```";
        return makeResult(ctx->IDENTIFIER()->getSymbol(), ss.str());
    }

    // 3) Check extended type methods (Vec.push, Map.get, etc.)
    //    Resolve receiver type to substitute generic placeholders (_key, _val, _elem)
    std::string receiverTypeName;
    if (!receiverText.empty()) {
        auto* encFunc = findEnclosingFunction(tree, cursorLine);
        if (encFunc) {
            auto locals = collectLocals(encFunc, cursorLine);
            auto lit = locals.find(receiverText);
            if (lit != locals.end())
                receiverTypeName = lit->second.typeName;
        }
    }

    // Parse generic type args from receiver type name (e.g. "Map<string, int64>")
    auto substituteGeneric = [&](const std::string& placeholder) -> std::string {
        if (placeholder[0] != '_') return placeholder;
        if (receiverTypeName.empty()) return placeholder;

        auto ltPos = receiverTypeName.find('<');
        if (ltPos == std::string::npos) return placeholder;
        auto gtPos = receiverTypeName.rfind('>');
        if (gtPos == std::string::npos) return placeholder;

        std::string inner = receiverTypeName.substr(ltPos + 1, gtPos - ltPos - 1);

        // Split by ", " respecting nested generics
        std::vector<std::string> typeArgs;
        int depth = 0;
        size_t start = 0;
        for (size_t i = 0; i < inner.size(); i++) {
            if (inner[i] == '<') depth++;
            else if (inner[i] == '>') depth--;
            else if (inner[i] == ',' && depth == 0) {
                auto arg = inner.substr(start, i - start);
                // Trim whitespace
                size_t b = arg.find_first_not_of(' ');
                size_t e = arg.find_last_not_of(' ');
                if (b != std::string::npos)
                    typeArgs.push_back(arg.substr(b, e - b + 1));
                start = i + 1;
            }
        }
        {
            auto arg = inner.substr(start);
            size_t b = arg.find_first_not_of(' ');
            size_t e = arg.find_last_not_of(' ');
            if (b != std::string::npos)
                typeArgs.push_back(arg.substr(b, e - b + 1));
        }

        // Determine base name to figure out arity
        std::string baseName = receiverTypeName.substr(0, ltPos);

        if (placeholder == "_elem" && !typeArgs.empty())
            return typeArgs[0];
        if (placeholder == "_key" && !typeArgs.empty())
            return typeArgs[0];
        if (placeholder == "_val" && typeArgs.size() >= 2)
            return typeArgs[1];
        if (placeholder == "_val" && typeArgs.size() == 1)
            return typeArgs[0];  // Vec<T>.pop() returns T

        return placeholder;
    };

    // Determine receiver base name for prioritized matching
    std::string receiverBaseName;
    if (!receiverTypeName.empty()) {
        auto ltPos = receiverTypeName.find('<');
        receiverBaseName = (ltPos != std::string::npos)
                            ? receiverTypeName.substr(0, ltPos)
                            : receiverTypeName;
    }

    for (auto& extName : {"Vec", "Map", "Set", "Task"}) {
        // If we know the receiver type, skip non-matching extended types
        if (!receiverBaseName.empty() && receiverBaseName != extName)
            continue;
        auto* desc = extTypeRegistry_.lookup(extName);
        if (!desc) continue;
        for (auto& m : desc->methods) {
            if (m.name == methodName) {
                std::ostringstream ss;
                std::string retType = substituteGeneric(m.returnType);
                ss << "```lux\n(" << extName << " method) " << retType
                   << " " << methodName << "(";
                for (size_t i = 0; i < m.paramTypes.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << substituteGeneric(m.paramTypes[i]);
                }
                ss << ")\n```";
                return makeResult(ctx->IDENTIFIER()->getSymbol(), ss.str());
            }
        }
    }

    // Fallback: if receiver was known but no method matched, try all extended types
    if (!receiverBaseName.empty()) {
        for (auto& extName : {"Vec", "Map", "Set", "Task"}) {
            if (receiverBaseName == extName) continue;  // already tried
            auto* desc = extTypeRegistry_.lookup(extName);
            if (!desc) continue;
            for (auto& m : desc->methods) {
                if (m.name == methodName) {
                    std::ostringstream ss;
                    ss << "```lux\n(" << extName << " method) " << m.returnType
                       << " " << methodName << "(";
                    for (size_t i = 0; i < m.paramTypes.size(); i++) {
                        if (i > 0) ss << ", ";
                        ss << m.paramTypes[i];
                    }
                    ss << ")\n```";
                    return makeResult(ctx->IDENTIFIER()->getSymbol(), ss.str());
                }
            }
        }
    }

    // 4) Check user extend block methods (local + cross-file)
    auto formatExtendMethod = [&](LuxParser::ExtendDeclContext* ext,
                                   LuxParser::ExtendMethodContext* m) -> std::optional<HoverResult> {
        std::ostringstream ss;
        ss << "```lux\n(" << ext->IDENTIFIER()->getText()
           << " method) " << typeSpecToString(m->typeSpec())
           << " " << methodName << "(";
        if (m->AMPERSAND()) {
            ss << "&self";
            for (auto* p : m->param()) {
                ss << ", " << typeSpecToString(p->typeSpec())
                   << " " << p->IDENTIFIER()->getText();
            }
        } else {
            if (auto* params = m->paramList()) {
                bool first = true;
                for (auto* p : params->param()) {
                    if (!first) ss << ", ";
                    first = false;
                    ss << typeSpecToString(p->typeSpec())
                       << " " << p->IDENTIFIER()->getText();
                }
            }
        }
        ss << ")\n```";
        return makeResult(ctx->IDENTIFIER()->getSymbol(), ss.str());
    };

    // Local file extend blocks
    for (auto* tld : tree->topLevelDecl()) {
        auto* ext = tld->extendDecl();
        if (!ext) continue;
        for (auto* m : ext->extendMethod()) {
            if (m->IDENTIFIER(0)->getText() == methodName)
                return formatExtendMethod(ext, m);
        }
    }

    // Cross-file extend blocks from project registry
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto syms = project->registry().getNamespaceSymbols(ns);
            for (auto* sym : syms) {
                if (sym->kind != ExportedSymbol::ExtendBlock) continue;
                auto* ext = dynamic_cast<LuxParser::ExtendDeclContext*>(sym->decl);
                if (!ext) continue;
                for (auto* m : ext->extendMethod()) {
                    if (m->IDENTIFIER(0)->getText() == methodName)
                        return formatExtendMethod(ext, m);
                }
            }
        }
    }

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Hover on field access: expr.field
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hoverFieldAccess(
        LuxParser::FieldAccessExprContext* ctx,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        size_t cursorLine,
        const ProjectContext* project) {
    std::string fieldName = ctx->IDENTIFIER()->getText();
    auto* receiver = ctx->expression();
    auto* token = ctx->IDENTIFIER()->getSymbol();

    // Try to find the receiver's type name
    std::string receiverTypeName;
    if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(receiver)) {
        std::string varName = ident->IDENTIFIER()->getText();
        // Look up local variable type
        auto* func = findEnclosingFunction(tree, cursorLine);
        if (func) {
            auto locals = collectLocals(func, cursorLine);
            auto it = locals.find(varName);
            if (it != locals.end()) {
                receiverTypeName = it->second.typeName;
            }
        }
    }

    // Search user structs for this field
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* sd = tld->structDecl()) {
            std::string sName = sd->IDENTIFIER()->getText();
            if (!receiverTypeName.empty() && sName != receiverTypeName)
                continue;
            for (auto* f : sd->structField()) {
                if (f->IDENTIFIER()->getText() == fieldName) {
                    std::string md = "```lux\n(field) " + typeSpecToString(f->typeSpec())
                                   + " " + sName + "." + fieldName + "\n```";
                    return makeResult(token, md);
                }
            }
        }
    }

    // Search user unions for this field
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* ud = tld->unionDecl()) {
            std::string uName = ud->IDENTIFIER()->getText();
            if (!receiverTypeName.empty() && uName != receiverTypeName)
                continue;
            for (auto* f : ud->unionField()) {
                if (f->IDENTIFIER()->getText() == fieldName) {
                    std::string md = "```lux\n(field) " + typeSpecToString(f->typeSpec())
                                   + " " + uName + "." + fieldName + "\n```";
                    return makeResult(token, md);
                }
            }
        }
    }

    // Search built-in structs from TypeRegistry (e.g. Error)
    if (!receiverTypeName.empty()) {
        auto* ti = typeRegistry_.lookup(receiverTypeName);
        if (ti && ti->kind == TypeKind::Struct) {
            for (auto& f : ti->fields) {
                if (f.name == fieldName) {
                    std::string md = "```lux\n(field) " + f.typeInfo->name
                                   + " " + receiverTypeName + "." + fieldName + "\n```";
                    return makeResult(token, md);
                }
            }
        }
    }

    // Search C structs for this field
    if (!receiverTypeName.empty()) {
        auto* cs = bindings.findStruct(receiverTypeName);
        if (cs) {
            for (auto& f : cs->fields) {
                if (f.name == fieldName) {
                    std::string md = "```c\n(field) " + f.typeInfo->name + " "
                                   + receiverTypeName + "." + fieldName + "\n```\n*C struct field*";
                    return makeResult(token, md);
                }
            }
        }
    } else {
        // No receiver type known — search all C structs
        for (auto& [sName, cs] : bindings.structs()) {
            for (auto& f : cs.fields) {
                if (f.name == fieldName) {
                    std::string md = "```c\n(field) " + f.typeInfo->name + " "
                                   + sName + "." + fieldName + "\n```\n*C struct field*";
                    return makeResult(token, md);
                }
            }
        }
    }

    // Search project registry for cross-file struct fields
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto syms = project->registry().getNamespaceSymbols(ns);
            for (auto* sym : syms) {
                if (sym->kind != ExportedSymbol::Struct) continue;
                auto* sd = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl);
                if (!sd) continue;
                if (!receiverTypeName.empty() &&
                    sd->IDENTIFIER()->getText() != receiverTypeName)
                    continue;
                for (auto* f : sd->structField()) {
                    if (f->IDENTIFIER()->getText() == fieldName) {
                        std::string md = "```lux\n(field) " + typeSpecToString(f->typeSpec())
                                       + " " + sd->IDENTIFIER()->getText() + "." + fieldName + "\n```";
                        return makeResult(token, md);
                    }
                }
            }
        }
    }

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Hover on enum access: Type::Variant
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hoverEnumAccess(
        LuxParser::EnumAccessExprContext* ctx,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        const ProjectContext* project) {
    auto ids = ctx->IDENTIFIER();
    if (ids.size() < 2) return std::nullopt;

    std::string typeName    = ids[0]->getText();
    std::string variantName = ids[1]->getText();
    auto* token = ids[1]->getSymbol();

    // User enum (local file)
    auto* ed = findEnumDecl(tree, typeName);
    if (ed) {
        std::string md = "```lux\n(variant) " + typeName + "::" + variantName + "\n```";
        return makeResult(token, md);
    }

    // C enum
    auto* ce = bindings.findEnum(typeName);
    if (ce) {
        for (auto& [vname, val] : ce->values) {
            if (vname == variantName) {
                std::string md = "```c\n(variant) " + typeName + "::" + variantName
                               + " = " + std::to_string(val) + "\n```\n*C enum variant*";
                return makeResult(token, md);
            }
        }
    }

    // Cross-file enum from project registry
    if (project && project->isValid()) {
        for (auto& ns : project->registry().allNamespaces()) {
            auto* sym = project->registry().findSymbol(ns, typeName);
            if (!sym || sym->kind != ExportedSymbol::Enum) continue;
            auto* enumCtx = dynamic_cast<LuxParser::EnumDeclContext*>(sym->decl);
            if (!enumCtx) continue;
            auto enumIds = enumCtx->IDENTIFIER();
            for (size_t i = 1; i < enumIds.size(); i++) {
                if (enumIds[i]->getText() == variantName) {
                    std::string md = "```lux\n(variant) " + typeName + "::" + variantName + "\n```";
                    return makeResult(token, md);
                }
            }
        }
    }

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Hover on static method call: Type::method(args)
// ═══════════════════════════════════════════════════════════════════════

std::optional<HoverResult> HoverProvider::hoverStaticMethodCall(
        LuxParser::StaticMethodCallExprContext* ctx,
        LuxParser::ProgramContext* tree,
        const CBindings& bindings,
        size_t cursorLine,
        const ProjectContext* project) {
    auto ids = ctx->IDENTIFIER();
    if (ids.size() < 2) return std::nullopt;

    std::string typeName   = ids[0]->getText();
    std::string methodName = ids[1]->getText();
    auto* token = ids[1]->getSymbol();

    // Check user extend block static methods
    for (auto* tld : tree->topLevelDecl()) {
        auto* ext = tld->extendDecl();
        if (!ext) continue;
        if (ext->IDENTIFIER()->getText() != typeName) continue;
        for (auto* m : ext->extendMethod()) {
            if (m->IDENTIFIER(0)->getText() == methodName) {
                std::ostringstream ss;
                ss << "```lux\n(static method) " << typeSpecToString(m->typeSpec())
                   << " " << typeName << "::" << methodName << "(";
                if (auto* params = m->paramList()) {
                    bool first = true;
                    for (auto* p : params->param()) {
                        if (!first) ss << ", ";
                        first = false;
                        ss << typeSpecToString(p->typeSpec())
                           << " " << p->IDENTIFIER()->getText();
                    }
                }
                ss << ")\n```";
                return makeResult(token, ss.str());
            }
        }
    }

    // Check extended type methods (Vec::new, Map::new, etc.)
    auto* extDesc = extTypeRegistry_.lookup(typeName);
    if (extDesc) {
        for (auto& m : extDesc->methods) {
            if (m.name == methodName) {
                std::ostringstream ss;
                ss << "```lux\n(static method) " << m.returnType << " "
                   << typeName << "::" << methodName << "(";
                for (size_t i = 0; i < m.paramTypes.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << m.paramTypes[i];
                }
                ss << ")\n```";
                return makeResult(token, ss.str());
            }
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
                    if (m->IDENTIFIER(0)->getText() == methodName) {
                        std::ostringstream ss;
                        ss << "```lux\n(static method) " << typeSpecToString(m->typeSpec())
                           << " " << typeName << "::" << methodName << "(";
                        if (auto* params = m->paramList()) {
                            bool first = true;
                            for (auto* p : params->param()) {
                                if (!first) ss << ", ";
                                first = false;
                                ss << typeSpecToString(p->typeSpec())
                                   << " " << p->IDENTIFIER()->getText();
                            }
                        }
                        ss << ")\n```";
                        return makeResult(token, ss.str());
                    }
                }
            }
        }
    }

    return std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════
//  Local variable collection
// ═══════════════════════════════════════════════════════════════════════

static void collectLocalsFromBlock(
        LuxParser::BlockContext* block,
        size_t beforeLine,
        std::unordered_map<std::string, HoverProvider::LocalVar>& out);

static void collectLocalsFromStmts(
        const std::vector<LuxParser::StatementContext*>& stmts,
        size_t beforeLine,
        std::unordered_map<std::string, HoverProvider::LocalVar>& out) {
    for (auto* stmt : stmts) {
        // Stop collecting if statement starts after the cursor line
        auto* start = stmt->getStart();
        if (start && start->getLine() > beforeLine + 1) break;

        if (auto* vd = stmt->varDeclStmt()) {
            std::string typeName = "";
            if (vd->typeSpec()) {
                typeName = vd->typeSpec()->getText();
            }
            std::string varName = vd->IDENTIFIER()->getText();
            out[varName] = {typeName, 0};
        }

        // Recurse into nested blocks
        if (auto* ifS = stmt->ifStmt()) {
            collectLocalsFromBlock(ifS->block(), beforeLine, out);
            for (auto* elif : ifS->elseIfClause())
                collectLocalsFromBlock(elif->block(), beforeLine, out);
            if (ifS->elseClause())
                collectLocalsFromBlock(ifS->elseClause()->block(), beforeLine, out);
        }
        if (auto* forIn = stmt->forStmt()) {
            if (auto* fin = dynamic_cast<LuxParser::ForInStmtContext*>(forIn)) {
                std::string tname = fin->typeSpec()->getText();
                out[fin->IDENTIFIER()->getText()] = {tname, 0};
                collectLocalsFromBlock(fin->block(), beforeLine, out);
            }
            if (auto* fc = dynamic_cast<LuxParser::ForClassicStmtContext*>(forIn)) {
                std::string tname = fc->typeSpec()->getText();
                out[fc->IDENTIFIER()->getText()] = {tname, 0};
                collectLocalsFromBlock(fc->block(), beforeLine, out);
            }
        }
        if (auto* ws = stmt->whileStmt())
            collectLocalsFromBlock(ws->block(), beforeLine, out);
        if (auto* dw = stmt->doWhileStmt())
            collectLocalsFromBlock(dw->block(), beforeLine, out);
        if (auto* ls = stmt->loopStmt())
            collectLocalsFromBlock(ls->block(), beforeLine, out);
        if (auto* sw = stmt->switchStmt()) {
            for (auto* c : sw->caseClause())
                collectLocalsFromBlock(c->block(), beforeLine, out);
            if (sw->defaultClause())
                collectLocalsFromBlock(sw->defaultClause()->block(), beforeLine, out);
        }
        if (auto* tc = stmt->tryCatchStmt()) {
            collectLocalsFromBlock(tc->block(), beforeLine, out);
            for (auto* cc : tc->catchClause()) {
                out[cc->IDENTIFIER()->getText()] = {cc->typeSpec()->getText(), 0};
                collectLocalsFromBlock(cc->block(), beforeLine, out);
            }
            if (tc->finallyClause())
                collectLocalsFromBlock(tc->finallyClause()->block(), beforeLine, out);
        }
    }
}

static void collectLocalsFromBlock(
        LuxParser::BlockContext* block,
        size_t beforeLine,
        std::unordered_map<std::string, HoverProvider::LocalVar>& out) {
    if (!block) return;
    collectLocalsFromStmts(block->statement(), beforeLine, out);
}

std::unordered_map<std::string, HoverProvider::LocalVar>
HoverProvider::collectLocals(LuxParser::FunctionDeclContext* func,
                             size_t beforeLine) {
    std::unordered_map<std::string, LocalVar> result;

    // Collect parameters
    if (auto* params = func->paramList()) {
        for (auto* p : params->param()) {
            std::string typeName = p->typeSpec()->getText();
            std::string paramName = p->IDENTIFIER()->getText();
            result[paramName] = {typeName, 0};
        }
    }

    // Collect locals from function body
    collectLocalsFromBlock(func->block(), beforeLine, result);
    return result;
}

// ═══════════════════════════════════════════════════════════════════════
//  AST lookup helpers
// ═══════════════════════════════════════════════════════════════════════

LuxParser::FunctionDeclContext*
HoverProvider::findEnclosingFunction(LuxParser::ProgramContext* tree,
                                     size_t line) {
    size_t tokenLine = line + 1; // convert to 1-based
    for (auto* tld : tree->topLevelDecl()) {
        auto* func = tld->functionDecl();
        if (!func) continue;
        auto* startTok = func->getStart();
        auto* stopTok  = func->getStop();
        if (!startTok || !stopTok) continue;
        if (tokenLine >= startTok->getLine() && tokenLine <= stopTok->getLine())
            return func;
    }
    return nullptr;
}

LuxParser::FunctionDeclContext*
HoverProvider::findFunctionDecl(LuxParser::ProgramContext* tree,
                                const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* func = tld->functionDecl()) {
            if (func->IDENTIFIER()->getText() == name)
                return func;
        }
    }
    return nullptr;
}

LuxParser::StructDeclContext*
HoverProvider::findStructDecl(LuxParser::ProgramContext* tree,
                              const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* sd = tld->structDecl()) {
            if (sd->IDENTIFIER()->getText() == name)
                return sd;
        }
    }
    return nullptr;
}

LuxParser::EnumDeclContext*
HoverProvider::findEnumDecl(LuxParser::ProgramContext* tree,
                            const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* ed = tld->enumDecl()) {
            if (ed->IDENTIFIER()[0]->getText() == name)
                return ed;
        }
    }
    return nullptr;
}

LuxParser::UnionDeclContext*
HoverProvider::findUnionDecl(LuxParser::ProgramContext* tree,
                             const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* ud = tld->unionDecl()) {
            if (ud->IDENTIFIER()->getText() == name)
                return ud;
        }
    }
    return nullptr;
}

LuxParser::TypeAliasDeclContext*
HoverProvider::findTypeAliasDecl(LuxParser::ProgramContext* tree,
                                 const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* ta = tld->typeAliasDecl()) {
            if (ta->IDENTIFIER()->getText() == name)
                return ta;
        }
    }
    return nullptr;
}

LuxParser::ExtendDeclContext*
HoverProvider::findExtendDecl(LuxParser::ProgramContext* tree,
                              const std::string& name) {
    for (auto* tld : tree->topLevelDecl()) {
        if (auto* ext = tld->extendDecl()) {
            if (ext->IDENTIFIER()->getText() == name)
                return ext;
        }
    }
    return nullptr;
}

// ═══════════════════════════════════════════════════════════════════════
//  Formatting helpers
// ═══════════════════════════════════════════════════════════════════════

std::string HoverProvider::typeSpecToString(LuxParser::TypeSpecContext* ctx) {
    if (!ctx) return "?";
    // Use the raw text from the parse tree — preserves the original syntax.
    return ctx->getText();
}

std::string HoverProvider::formatFunctionDecl(LuxParser::FunctionDeclContext* func) {
    std::ostringstream ss;
    ss << "```lux\n";
    ss << typeSpecToString(func->typeSpec()) << " " << func->IDENTIFIER()->getText() << "(";
    if (auto* params = func->paramList()) {
        bool first = true;
        for (auto* p : params->param()) {
            if (!first) ss << ", ";
            first = false;
            ss << typeSpecToString(p->typeSpec());
            if (p->SPREAD()) ss << " ...";
            else ss << " ";
            ss << p->IDENTIFIER()->getText();
        }
    }
    ss << ")\n```";
    return ss.str();
}

std::string HoverProvider::formatExternDecl(LuxParser::ExternDeclContext* ext) {
    std::ostringstream ss;
    ss << "```lux\nextern " << typeSpecToString(ext->typeSpec())
       << " " << ext->IDENTIFIER()->getText() << "(";
    if (auto* params = ext->externParamList()) {
        bool first = true;
        for (auto* ep : params->externParam()) {
            if (!first) ss << ", ";
            first = false;
            ss << typeSpecToString(ep->typeSpec());
            if (ep->IDENTIFIER())
                ss << " " << ep->IDENTIFIER()->getText();
        }
    }
    if (ext->SPREAD()) {
        if (ext->externParamList()) ss << ", ";
        ss << "...";
    }
    ss << ")\n```";
    return ss.str();
}

std::string HoverProvider::formatStructDecl(LuxParser::StructDeclContext* decl) {
    std::ostringstream ss;
    ss << "```lux\nstruct " << decl->IDENTIFIER()->getText() << " {\n";
    for (auto* f : decl->structField()) {
        ss << "    " << typeSpecToString(f->typeSpec()) << " "
           << f->IDENTIFIER()->getText() << "\n";
    }
    ss << "}\n```";

    // Show extend methods if available
    // (We don't have access to tree here, but the caller can append if needed)
    return ss.str();
}

std::string HoverProvider::formatEnumDecl(LuxParser::EnumDeclContext* decl) {
    auto ids = decl->IDENTIFIER();
    std::ostringstream ss;
    ss << "```lux\nenum " << ids[0]->getText() << " {\n";
    for (size_t i = 1; i < ids.size(); i++) {
        ss << "    " << ids[i]->getText();
        if (i + 1 < ids.size()) ss << ",";
        ss << "\n";
    }
    ss << "}\n```";
    return ss.str();
}

std::string HoverProvider::formatUnionDecl(LuxParser::UnionDeclContext* decl) {
    std::ostringstream ss;
    ss << "```lux\nunion " << decl->IDENTIFIER()->getText() << " {\n";
    for (auto* f : decl->unionField()) {
        ss << "    " << typeSpecToString(f->typeSpec()) << " "
           << f->IDENTIFIER()->getText() << "\n";
    }
    ss << "}\n```";
    return ss.str();
}

std::string HoverProvider::formatCFunction(const CFunction& func) {
    std::ostringstream ss;
    ss << "```c\n" << func.returnType->name << " " << func.name << "(";
    for (size_t i = 0; i < func.paramTypes.size(); i++) {
        if (i > 0) ss << ", ";
        ss << func.paramTypes[i]->name;
    }
    if (func.isVariadic) {
        if (!func.paramTypes.empty()) ss << ", ";
        ss << "...";
    }
    ss << ")\n```\n*C function*";
    return ss.str();
}

std::string HoverProvider::formatCStruct(const std::string& name,
                                          const CStruct& s) {
    std::ostringstream ss;
    ss << "```c\nstruct " << name << " {\n";
    for (auto& f : s.fields) {
        ss << "    " << f.typeInfo->name << " " << f.name << "\n";
    }
    ss << "}\n```\n*C struct*";
    return ss.str();
}

std::string HoverProvider::formatCEnum(const std::string& name,
                                        const CEnum& e) {
    std::ostringstream ss;
    ss << "```c\nenum " << name << " {\n";
    size_t count = 0;
    for (auto& [vname, val] : e.values) {
        ss << "    " << vname << " = " << val;
        if (count + 1 < e.values.size()) ss << ",";
        ss << "\n";
        count++;
        // Limit to 20 variants for large enums
        if (count >= 20 && e.values.size() > 20) {
            ss << "    // ... " << (e.values.size() - 20) << " more\n";
            break;
        }
    }
    ss << "}\n```\n*C enum*";
    return ss.str();
}

std::string HoverProvider::formatBuiltinSignature(const BuiltinSignature& sig) {
    std::ostringstream ss;
    ss << "```lux\n(builtin) " << sig.returnType << " " << sig.name << "(";
    for (size_t i = 0; i < sig.paramTypes.size(); i++) {
        if (i > 0) ss << ", ";
        ss << sig.paramTypes[i];
    }
    if (sig.isVariadic) {
        if (!sig.paramTypes.empty()) ss << ", ";
        ss << "...";
    }
    ss << ")\n```";
    return ss.str();
}

std::string HoverProvider::formatTypeInfo(const TypeInfo* ti) {
    if (!ti) return "?";
    return ti->name;
}

std::string HoverProvider::formatExtendMethods(LuxParser::ExtendDeclContext* ext) {
    std::ostringstream ss;
    ss << "```lux\nextend " << ext->IDENTIFIER()->getText() << " {\n";
    for (auto* m : ext->extendMethod()) {
        ss << "    " << typeSpecToString(m->typeSpec()) << " "
           << m->IDENTIFIER(0)->getText() << "(";

        // Check if first param is &self
        if (m->AMPERSAND()) {
            // Format: typeSpec name(&self, params...)
            auto ids = m->IDENTIFIER();
            // ids[0] is method name, the &self param name is separate
            ss << "&self";
            if (auto* params = m->paramList()) {
                for (auto* p : params->param()) {
                    ss << ", " << typeSpecToString(p->typeSpec())
                       << " " << p->IDENTIFIER()->getText();
                }
            }
        } else {
            // Static method
            if (auto* params = m->paramList()) {
                bool first = true;
                for (auto* p : params->param()) {
                    if (!first) ss << ", ";
                    first = false;
                    ss << typeSpecToString(p->typeSpec())
                       << " " << p->IDENTIFIER()->getText();
                }
            }
        }
        ss << ")\n";
    }
    ss << "}\n```";
    return ss.str();
}

HoverResult HoverProvider::makeResult(antlr4::Token* token,
                                       const std::string& contents) {
    HoverResult r;
    r.contents = contents;
    r.line     = token->getLine() - 1; // to 0-based
    r.col      = token->getCharPositionInLine();
    r.endLine  = r.line;
    r.endCol   = r.col + token->getText().size();
    return r;
}
