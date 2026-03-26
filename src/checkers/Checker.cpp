#include "checkers/Checker.h"
#include "generated/LuxLexer.h"
#include "ffi/CBindings.h"

// ═══════════════════════════════════════════════════════════════════════
//  Error helper — attaches line:col to every diagnostic
// ═══════════════════════════════════════════════════════════════════════

void Checker::error(antlr4::ParserRuleContext* ctx, const std::string& msg) {
    if (ctx && ctx->getStart()) {
        auto line = ctx->getStart()->getLine();
        auto col  = ctx->getStart()->getCharPositionInLine() + 1;
        errors_.push_back(std::to_string(line) + ":" +
                          std::to_string(col) + ": " + msg);
    } else {
        errors_.push_back(msg);
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Main entry point
// ═══════════════════════════════════════════════════════════════════════

bool Checker::check(LuxParser::ProgramContext* tree) {
    if (!tree) {
        errors_.push_back("empty program");
        return false;
    }

    // Pass 0: register global builtins (always available, no import needed)
    registerGlobalBuiltins();

    // Pass 0.5: register C header bindings (from #include directives)
    if (cBindings_) {
        // Register C structs as types
        for (auto& [name, cstruct] : cBindings_->structs()) {
            if (!typeRegistry_.lookup(name)) {
                TypeInfo ti;
                ti.name = name;
                ti.kind = TypeKind::Struct;
                ti.bitWidth = 0;
                ti.isSigned = false;
                ti.fields = cstruct.fields;
                typeRegistry_.registerType(std::move(ti));
            }
        }

        // Register C enums as types + enum constant globals
        for (auto& [name, cenum] : cBindings_->enums()) {
            if (!typeRegistry_.lookup(name)) {
                TypeInfo ti;
                ti.name = name;
                ti.kind = TypeKind::Enum;
                ti.bitWidth = 32;
                ti.isSigned = false;
                ti.builtinSuffix = "i32";
                for (auto& [vname, val] : cenum.values)
                    ti.enumVariants.push_back(vname);
                typeRegistry_.registerType(std::move(ti));
            }
            // Register each enum constant as a known variable
            for (auto& [vname, val] : cenum.values) {
                auto* enumTI = typeRegistry_.lookup(name);
                if (enumTI)
                    cEnumConstants_[vname] = { enumTI, val };
            }
        }

        // Register C typedefs as type aliases
        for (auto& [name, ctdef] : cBindings_->typedefs()) {
            if (!typeRegistry_.lookup(name)) {
                TypeInfo ti = *ctdef.underlying;
                ti.name = name;
                typeRegistry_.registerType(std::move(ti));
            }
        }

        // Register C functions
        for (auto& [name, cfunc] : cBindings_->functions()) {
            auto* funcType = makeFunctionType(
                cfunc.returnType, cfunc.paramTypes, cfunc.isVariadic);
            functions_[name] = funcType;
            globalBuiltins_.insert(name);
        }

        // Register C #define integer constants
        for (auto& [name, cmacro] : cBindings_->macros()) {
            auto* int32TI = typeRegistry_.lookup("int32");
            if (int32TI)
                cEnumConstants_[name] = { int32TI, cmacro.value };
        }

        // Register C #define struct literal constants (e.g. RAYWHITE)
        for (auto& [name, sm] : cBindings_->structMacros()) {
            auto* structTI = typeRegistry_.lookup(sm.structType);
            if (structTI)
                cEnumConstants_[name] = { structTI, 0 };
        }

        // Register C global variables
        for (auto& [name, gvar] : cBindings_->globals()) {
            cGlobals_[name] = gvar.type;
        }
    }

    // Pass 1: register all `use` declarations
    checkUseDecls(tree);

    // Pass 1.5: register cross-file structs, enums, and functions
    // from same namespace and user imports, so type resolution works.
    if (nsRegistry_) {
        // Same-namespace external symbols
        auto extSyms = nsRegistry_->getExternalSymbols(
            currentNamespace_, currentFile_);
        for (auto* sym : extSyms) {
            if (sym->kind == ExportedSymbol::Struct) {
                auto* decl = static_cast<LuxParser::StructDeclContext*>(sym->decl);
                checkStructDecl(decl);
            } else if (sym->kind == ExportedSymbol::Union) {
                auto* decl = static_cast<LuxParser::UnionDeclContext*>(sym->decl);
                checkUnionDecl(decl);
            } else if (sym->kind == ExportedSymbol::Enum) {
                auto* decl = static_cast<LuxParser::EnumDeclContext*>(sym->decl);
                checkEnumDecl(decl);
            } else if (sym->kind == ExportedSymbol::TypeAlias) {
                auto* decl = static_cast<LuxParser::TypeAliasDeclContext*>(sym->decl);
                checkTypeAliasDecl(decl);
            } else if (sym->kind == ExportedSymbol::Function) {
                auto* decl = static_cast<LuxParser::FunctionDeclContext*>(sym->decl);
                registerFunctionSignature(decl);
            }
        }

        // User-imported symbols from other namespaces
        for (auto& [symName, ns] : userImports_) {
            auto* sym = nsRegistry_->findSymbol(ns, symName);
            if (!sym) continue;
            if (sym->kind == ExportedSymbol::Struct) {
                auto* decl = static_cast<LuxParser::StructDeclContext*>(sym->decl);
                checkStructDecl(decl);
            } else if (sym->kind == ExportedSymbol::Union) {
                auto* decl = static_cast<LuxParser::UnionDeclContext*>(sym->decl);
                checkUnionDecl(decl);
            } else if (sym->kind == ExportedSymbol::Enum) {
                auto* decl = static_cast<LuxParser::EnumDeclContext*>(sym->decl);
                checkEnumDecl(decl);
            } else if (sym->kind == ExportedSymbol::TypeAlias) {
                auto* decl = static_cast<LuxParser::TypeAliasDeclContext*>(sym->decl);
                checkTypeAliasDecl(decl);
            } else if (sym->kind == ExportedSymbol::Function) {
                auto* decl = static_cast<LuxParser::FunctionDeclContext*>(sym->decl);
                registerFunctionSignature(decl);
            }
        }
    }

    // Pass 2: register type aliases
    for (auto* decl : tree->topLevelDecl()) {
        if (auto* ta = decl->typeAliasDecl()) {
            checkTypeAliasDecl(ta);
        }
    }

    // Pass 3: register struct and enum types
    for (auto* decl : tree->topLevelDecl()) {
        if (auto* sd = decl->structDecl()) {
            checkStructDecl(sd);
        } else if (auto* ud = decl->unionDecl()) {
            checkUnionDecl(ud);
        } else if (auto* ed = decl->enumDecl()) {
            checkEnumDecl(ed);
        }
    }

    // Pass 3.5: register struct methods via `extend` blocks
    for (auto* decl : tree->topLevelDecl()) {
        if (auto* ext = decl->extendDecl()) {
            checkExtendDecl(ext);
        }
    }

    // Pass 4: register function signatures (before checking bodies)
    for (auto* decl : tree->topLevelDecl()) {
        if (auto* func = decl->functionDecl()) {
            registerFunctionSignature(func);
        }
        if (auto* ext = decl->externDecl()) {
            checkExternDecl(ext);
        }
    }

    // Pass 5: check function bodies
    for (auto* decl : tree->topLevelDecl()) {
        if (auto* func = decl->functionDecl()) {
            locals_.clear();
            checkFunction(func);
        }
    }

    return errors_.empty();
}

// ═══════════════════════════════════════════════════════════════════════
//  Type resolution helpers
// ═══════════════════════════════════════════════════════════════════════

const TypeInfo* Checker::resolveTypeSpec(LuxParser::TypeSpecContext* ctx,
                                         unsigned& arrayDims) {
    arrayDims = 0;
    auto* cur = ctx;

    // Unwrap array dimensions: [][]T → arrayDims=2, [N]T → arrayDims=1 (fixed)
    while (cur->LBRACKET()) {
        arrayDims++;
        if (cur->INT_LIT()) {
            // Fixed-size array [N]T — validate size is positive
            int64_t size = std::stoll(cur->INT_LIT()->getText());
            if (size <= 0) {
                error(cur, "array size must be a positive integer, got " +
                      cur->INT_LIT()->getText());
                return nullptr;
            }
        }
        cur = cur->typeSpec(0);
    }

    // Pointer type: *T
    if (cur->STAR()) {
        unsigned innerDims = 0;
        auto* inner = resolveTypeSpec(cur->typeSpec(0), innerDims);
        if (!inner) return nullptr;
        return getPointerType(inner);
    }

    // Function type: fn(params) -> ret
    if (auto* fnSpec = cur->fnTypeSpec()) {
        auto specs = fnSpec->typeSpec();
        if (specs.empty()) return nullptr;

        unsigned retDims = 0;
        auto* retType = resolveTypeSpec(specs.back(), retDims);
        if (!retType) return nullptr;

        std::vector<const TypeInfo*> paramTypes;
        for (size_t i = 0; i + 1 < specs.size(); i++) {
            unsigned pDims = 0;
            auto* pType = resolveTypeSpec(specs[i], pDims);
            if (!pType) return nullptr;
            paramTypes.push_back(pType);
        }

        return makeFunctionType(retType, paramTypes);
    }

    // Generic extended type: Vec<int32>, Map<string, int32>, etc.
    if (cur->LT()) {
        auto baseName = cur->IDENTIFIER()->getText();
        auto* extDesc = extTypeRegistry_.lookup(baseName);
        if (!extDesc) {
            error(cur, "'" + baseName + "' is not a known generic type");
            return nullptr;
        }
        if (!imports_.isImported(baseName)) {
            error(cur, "type '" + baseName +
                       "' is not imported. Did you forget 'use std::collections::" +
                       baseName + ";'?");
            return nullptr;
        }
        auto typeParams = cur->typeSpec();
        if (typeParams.empty()) {
            error(cur, "generic type '" + baseName + "' requires type parameters");
            return nullptr;
        }

        if (extDesc->genericArity == 1) {
            // Single type param: Vec<T>
            if (typeParams.size() != 1) {
                error(cur, "'" + baseName + "' expects 1 type parameter, got " +
                           std::to_string(typeParams.size()));
                return nullptr;
            }
            unsigned elemDims = 0;
            auto* elemType = resolveTypeSpec(typeParams[0], elemDims);
            if (!elemType) return nullptr;

            auto fullName = baseName + "<" + elemType->name + ">";
            for (auto& dt : dynamicTypes_) {
                if (dt->name == fullName)
                    return dt.get();
            }
            auto ti = std::make_unique<TypeInfo>();
            ti->name = fullName;
            ti->kind = TypeKind::Extended;
            ti->bitWidth = 0;
            ti->isSigned = false;
            ti->builtinSuffix = elemType->builtinSuffix;
            ti->elementType = elemType;
            ti->extendedKind = baseName;
            const TypeInfo* raw = ti.get();
            dynamicTypes_.push_back(std::move(ti));
            return raw;
        } else if (extDesc->genericArity == 2) {
            // Two type params: Map<K, V>
            if (typeParams.size() != 2) {
                error(cur, "'" + baseName + "' expects 2 type parameters, got " +
                           std::to_string(typeParams.size()));
                return nullptr;
            }
            unsigned keyDims = 0, valDims = 0;
            auto* keyType = resolveTypeSpec(typeParams[0], keyDims);
            auto* valType = resolveTypeSpec(typeParams[1], valDims);
            if (!keyType || !valType) return nullptr;

            auto fullName = baseName + "<" + keyType->name + ", " + valType->name + ">";
            for (auto& dt : dynamicTypes_) {
                if (dt->name == fullName)
                    return dt.get();
            }
            auto ti = std::make_unique<TypeInfo>();
            ti->name = fullName;
            ti->kind = TypeKind::Extended;
            ti->bitWidth = 0;
            ti->isSigned = false;
            ti->builtinSuffix = keyType->builtinSuffix + "_" + valType->builtinSuffix;
            ti->keyType = keyType;
            ti->valueType = valType;
            ti->extendedKind = baseName;
            const TypeInfo* raw = ti.get();
            dynamicTypes_.push_back(std::move(ti));
            return raw;
        } else {
            error(cur, "unsupported generic arity for '" + baseName + "'");
            return nullptr;
        }
    }

    // Primitive or named type
    auto name = cur->getText();

    // cstring is a built-in alias for *char
    if (name == "cstring")
        return getPointerType(typeRegistry_.lookup("char"));

    auto* ti = typeRegistry_.lookup(name);
    if (!ti) {
        error(cur, "unknown type '" + name + "'");
    }
    return ti;
}

const TypeInfo* Checker::getPointerType(const TypeInfo* pointee) {
    for (auto& dt : dynamicTypes_) {
        if (dt->kind == TypeKind::Pointer && dt->pointeeType == pointee)
            return dt.get();
    }
    auto ti = std::make_unique<TypeInfo>();
    ti->name = "*" + pointee->name;
    ti->kind = TypeKind::Pointer;
    ti->bitWidth = 0;
    ti->isSigned = false;
    ti->builtinSuffix = "ptr";
    ti->pointeeType = pointee;
    const TypeInfo* raw = ti.get();
    dynamicTypes_.push_back(std::move(ti));
    return raw;
}

const TypeInfo* Checker::makeFunctionType(const TypeInfo* returnType,
                                           const std::vector<const TypeInfo*>& paramTypes,
                                           bool isVariadic) {
    auto ti = std::make_unique<TypeInfo>();
    ti->kind = TypeKind::Function;
    ti->bitWidth = 0;
    ti->isSigned = false;
    ti->builtinSuffix = "ptr";
    ti->returnType = returnType;
    ti->paramTypes = paramTypes;
    ti->isVariadic = isVariadic;

    ti->name = "fn(";
    for (size_t i = 0; i < paramTypes.size(); i++) {
        if (i > 0) ti->name += ",";
        if (isVariadic && i == paramTypes.size() - 1)
            ti->name += "...";
        ti->name += paramTypes[i]->name;
    }
    ti->name += ")->" + returnType->name;

    const TypeInfo* raw = ti.get();
    dynamicTypes_.push_back(std::move(ti));
    return raw;
}

std::string Checker::resolveBaseTypeName(LuxParser::TypeSpecContext* ctx) {
    while (!ctx->typeSpec().empty())
        ctx = ctx->typeSpec(0);
    auto text = ctx->getText();
    while (!text.empty() && text[0] == '*')
        text = text.substr(1);
    return text;
}

const TypeInfo* Checker::resolveBuiltinReturnType(const std::string& retName) {
    // Handle "Vec<T>" return types from builtins
    if (retName.size() > 4 && retName.substr(0, 4) == "Vec<" && retName.back() == '>') {
        std::string elemName = retName.substr(4, retName.size() - 5);
        auto* elemType = typeRegistry_.lookup(elemName);
        if (!elemType) return nullptr;

        auto fullName = retName;
        for (auto& dt : dynamicTypes_) {
            if (dt->name == fullName)
                return dt.get();
        }
        auto ti = std::make_unique<TypeInfo>();
        ti->name = fullName;
        ti->kind = TypeKind::Extended;
        ti->bitWidth = 0;
        ti->isSigned = false;
        ti->builtinSuffix = elemType->builtinSuffix;
        ti->elementType = elemType;
        ti->extendedKind = "Vec";
        const TypeInfo* raw = ti.get();
        dynamicTypes_.push_back(std::move(ti));
        return raw;
    }
    return typeRegistry_.lookup(retName);
}

// ═══════════════════════════════════════════════════════════════════════
//  Type query helpers
// ═══════════════════════════════════════════════════════════════════════

bool Checker::isNumeric(const TypeInfo* ti) {
    return ti && (ti->kind == TypeKind::Integer || ti->kind == TypeKind::Float);
}

bool Checker::isInteger(const TypeInfo* ti) {
    return ti && ti->kind == TypeKind::Integer;
}

bool Checker::isAssignable(const TypeInfo* lhs, const TypeInfo* rhs) {
    if (!lhs || !rhs) return true;
    if (lhs == rhs) return true;
    if (lhs->name == rhs->name) return true;

    // Integer ↔ integer (implicit widening/narrowing)
    if (lhs->kind == TypeKind::Integer && rhs->kind == TypeKind::Integer)
        return true;

    // Enum ↔ integer (C enums are integers)
    if (lhs->kind == TypeKind::Enum && rhs->kind == TypeKind::Integer)
        return true;
    if (lhs->kind == TypeKind::Integer && rhs->kind == TypeKind::Enum)
        return true;
    if (lhs->kind == TypeKind::Enum && rhs->kind == TypeKind::Enum)
        return true;

    // Float ↔ float
    if (lhs->kind == TypeKind::Float && rhs->kind == TypeKind::Float)
        return true;

    // Function ↔ function
    if (lhs->kind == TypeKind::Function && rhs->kind == TypeKind::Function)
        return true;

    // Function pointer ↔ function (C callback decay)
    if (lhs->kind == TypeKind::Pointer && lhs->pointeeType &&
        lhs->pointeeType->kind == TypeKind::Function &&
        rhs->kind == TypeKind::Function)
        return true;
    if (rhs->kind == TypeKind::Pointer && rhs->pointeeType &&
        rhs->pointeeType->kind == TypeKind::Function &&
        lhs->kind == TypeKind::Function)
        return true;

    // Same kind
    if (lhs->kind == rhs->kind) return true;

    return false;
}

// ═══════════════════════════════════════════════════════════════════════
//  Expression type resolution
// ═══════════════════════════════════════════════════════════════════════

const TypeInfo* Checker::resolveExprType(LuxParser::ExpressionContext* expr) {
    if (!expr) return nullptr;

    // ── Literals ──────────────────────────────────────────────────────
    if (dynamic_cast<LuxParser::IntLitExprContext*>(expr))
        return typeRegistry_.lookup("int32");

    if (dynamic_cast<LuxParser::FloatLitExprContext*>(expr))
        return typeRegistry_.lookup("float64");

    if (dynamic_cast<LuxParser::BoolLitExprContext*>(expr))
        return typeRegistry_.lookup("bool");

    if (dynamic_cast<LuxParser::CharLitExprContext*>(expr))
        return typeRegistry_.lookup("char");

    if (dynamic_cast<LuxParser::StrLitExprContext*>(expr))
        return typeRegistry_.lookup("string");

    // C string literal: c"hello" → *char (null-terminated)
    if (dynamic_cast<LuxParser::CStrLitExprContext*>(expr))
        return getPointerType(typeRegistry_.lookup("char"));

    if (dynamic_cast<LuxParser::NullLitExprContext*>(expr))
        return nullptr; // null is compatible with any pointer

    // ── Identifier ───────────────────────────────────────────────────
    if (auto* id = dynamic_cast<LuxParser::IdentExprContext*>(expr)) {
        auto name = id->IDENTIFIER()->getText();
        auto it = locals_.find(name);
        if (it != locals_.end())
            return it->second.type;

        auto fit = functions_.find(name);
        if (fit != functions_.end())
            return fit->second;

        // C enum constants from #include headers
        auto ceit = cEnumConstants_.find(name);
        if (ceit != cEnumConstants_.end())
            return ceit->second.type;

        // C global variables from #include headers
        auto cgit = cGlobals_.find(name);
        if (cgit != cGlobals_.end())
            return cgit->second;

        // Imported symbols — check constant registry first, then treat as
        // polymorphic function reference (nullptr).
        if (imports_.isImported(name)) {
            auto& constType = builtinRegistry_.lookupConstant(name);
            if (!constType.empty())
                return typeRegistry_.lookup(constType);
            return nullptr;
        }

        // User namespace import or same-namespace symbol
        if (userImports_.count(name)) return nullptr;
        if (nsRegistry_ && !currentNamespace_.empty()) {
            auto* sym = nsRegistry_->findSymbol(currentNamespace_, name);
            if (sym && sym->sourceFile != currentFile_)
                return nullptr;
        }

        error(expr, "undefined variable '" + name + "'");
        return nullptr;
    }

    // ── Parenthesized ────────────────────────────────────────────────
    if (auto* p = dynamic_cast<LuxParser::ParenExprContext*>(expr))
        return resolveExprType(p->expression());

    // ── Unary negation ───────────────────────────────────────────────
    if (auto* neg = dynamic_cast<LuxParser::NegExprContext*>(expr)) {
        auto* operand = resolveExprType(neg->expression());
        if (operand && !isNumeric(operand))
            error(expr, "unary '-' requires numeric operand, got '" +
                             operand->name + "'");
        return operand;
    }

    // ── Logical NOT ──────────────────────────────────────────────────
    if (auto* lnot = dynamic_cast<LuxParser::LogicalNotExprContext*>(expr)) {
        resolveExprType(lnot->expression());
        return typeRegistry_.lookup("bool");
    }

    // ── Bitwise NOT ──────────────────────────────────────────────────
    if (auto* bnot = dynamic_cast<LuxParser::BitNotExprContext*>(expr)) {
        auto* operand = resolveExprType(bnot->expression());
        if (operand && !isInteger(operand))
            error(expr, "operator '~' requires integer operand, got '" +
                             operand->name + "'");
        return operand;
    }

    // ── Multiplicative: *, /, % ──────────────────────────────────────
    if (auto* mul = dynamic_cast<LuxParser::MulExprContext*>(expr)) {
        auto exprs = mul->expression();
        auto* lhs = resolveExprType(exprs[0]);
        auto* rhs = resolveExprType(exprs[1]);
        auto opText = mul->op->getText();

        if (opText == "%") {
            if (lhs && !isInteger(lhs))
                error(expr, "operator '%' requires integer operands, got '" +
                                 lhs->name + "'");
            if (rhs && !isInteger(rhs))
                error(expr, "operator '%' requires integer operands, got '" +
                                 rhs->name + "'");
        } else {
            if (lhs && !isNumeric(lhs))
                error(expr, "operator '" + opText +
                                 "' requires numeric operands, got '" + lhs->name + "'");
            if (rhs && !isNumeric(rhs))
                error(expr, "operator '" + opText +
                                 "' requires numeric operands, got '" + rhs->name + "'");
        }

        if (lhs && rhs && lhs->kind == rhs->kind)
            return lhs->bitWidth >= rhs->bitWidth ? lhs : rhs;
        return lhs ? lhs : rhs;
    }

    // ── Additive: +, - ──────────────────────────────────────────────
    if (auto* add = dynamic_cast<LuxParser::AddSubExprContext*>(expr)) {
        auto exprs = add->expression();
        auto* lhs = resolveExprType(exprs[0]);
        auto* rhs = resolveExprType(exprs[1]);
        auto opText = add->op->getText();

        bool lhsPtr = lhs && lhs->kind == TypeKind::Pointer;
        bool rhsPtr = rhs && rhs->kind == TypeKind::Pointer;
        bool lhsNum = lhs && isNumeric(lhs);
        bool rhsNum = rhs && isNumeric(rhs);

        // pointer + int  or  int + pointer  → pointer
        if ((lhsPtr && rhsNum) || (lhsNum && rhsPtr)) {
            return lhsPtr ? lhs : rhs;
        }
        // pointer - pointer → i64 (ptrdiff)
        if (lhsPtr && rhsPtr && opText == "-") {
            return typeRegistry_.lookup("i64");
        }

        if (lhs && !isNumeric(lhs))
            error(expr, "operator '" + opText +
                             "' requires numeric operands, got '" + lhs->name + "'");
        if (rhs && !isNumeric(rhs))
            error(expr, "operator '" + opText +
                             "' requires numeric operands, got '" + rhs->name + "'");

        if (lhs && rhs && lhs->kind == rhs->kind)
            return lhs->bitWidth >= rhs->bitWidth ? lhs : rhs;
        return lhs ? lhs : rhs;
    }

    // ── Shift: <<, >> ───────────────────────────────────────────────
    if (auto* shift = dynamic_cast<LuxParser::ShiftExprContext*>(expr)) {
        auto exprs = shift->expression();
        auto* lhs = resolveExprType(exprs[0]);
        auto* rhs = resolveExprType(exprs[1]);
        auto opText = shift->op->getText();

        if (lhs && !isInteger(lhs))
            error(expr, "operator '" + opText +
                             "' requires integer operands, got '" + lhs->name + "'");
        if (rhs && !isInteger(rhs))
            error(expr, "operator '" + opText +
                             "' requires integer operands, got '" + rhs->name + "'");
        return lhs;
    }

    // ── Relational: <, >, <=, >= ────────────────────────────────────
    if (auto* rel = dynamic_cast<LuxParser::RelExprContext*>(expr)) {
        auto exprs = rel->expression();
        auto* lhs = resolveExprType(exprs[0]);
        auto* rhs = resolveExprType(exprs[1]);
        auto opText = rel->op->getText();

        if (lhs && !isNumeric(lhs))
            error(expr, "operator '" + opText +
                             "' requires numeric operands, got '" + lhs->name + "'");
        if (rhs && !isNumeric(rhs))
            error(expr, "operator '" + opText +
                             "' requires numeric operands, got '" + rhs->name + "'");
        return typeRegistry_.lookup("bool");
    }

    // ── Equality: ==, != ────────────────────────────────────────────
    if (auto* eq = dynamic_cast<LuxParser::EqExprContext*>(expr)) {
        auto exprs = eq->expression();
        resolveExprType(exprs[0]);
        resolveExprType(exprs[1]);
        return typeRegistry_.lookup("bool");
    }

    // ── Bitwise AND/XOR/OR ──────────────────────────────────────────
    if (auto* ba = dynamic_cast<LuxParser::BitAndExprContext*>(expr)) {
        auto exprs = ba->expression();
        auto* lhs = resolveExprType(exprs[0]);
        auto* rhs = resolveExprType(exprs[1]);
        if (lhs && !isInteger(lhs))
            error(expr, "operator '&' requires integer operands, got '" +
                             lhs->name + "'");
        if (rhs && !isInteger(rhs))
            error(expr, "operator '&' requires integer operands, got '" +
                             rhs->name + "'");
        if (lhs && rhs) return lhs->bitWidth >= rhs->bitWidth ? lhs : rhs;
        return lhs ? lhs : rhs;
    }

    if (auto* bx = dynamic_cast<LuxParser::BitXorExprContext*>(expr)) {
        auto exprs = bx->expression();
        auto* lhs = resolveExprType(exprs[0]);
        auto* rhs = resolveExprType(exprs[1]);
        if (lhs && !isInteger(lhs))
            error(expr, "operator '^' requires integer operands, got '" +
                             lhs->name + "'");
        if (rhs && !isInteger(rhs))
            error(expr, "operator '^' requires integer operands, got '" +
                             rhs->name + "'");
        if (lhs && rhs) return lhs->bitWidth >= rhs->bitWidth ? lhs : rhs;
        return lhs ? lhs : rhs;
    }

    if (auto* bo = dynamic_cast<LuxParser::BitOrExprContext*>(expr)) {
        auto exprs = bo->expression();
        auto* lhs = resolveExprType(exprs[0]);
        auto* rhs = resolveExprType(exprs[1]);
        if (lhs && !isInteger(lhs))
            error(expr, "operator '|' requires integer operands, got '" +
                             lhs->name + "'");
        if (rhs && !isInteger(rhs))
            error(expr, "operator '|' requires integer operands, got '" +
                             rhs->name + "'");
        if (lhs && rhs) return lhs->bitWidth >= rhs->bitWidth ? lhs : rhs;
        return lhs ? lhs : rhs;
    }

    // ── Logical AND/OR ──────────────────────────────────────────────
    if (auto* la = dynamic_cast<LuxParser::LogicalAndExprContext*>(expr)) {
        auto exprs = la->expression();
        resolveExprType(exprs[0]);
        resolveExprType(exprs[1]);
        return typeRegistry_.lookup("bool");
    }

    if (auto* lo = dynamic_cast<LuxParser::LogicalOrExprContext*>(expr)) {
        auto exprs = lo->expression();
        resolveExprType(exprs[0]);
        resolveExprType(exprs[1]);
        return typeRegistry_.lookup("bool");
    }

    // ── Pre-increment/decrement ─────────────────────────────────────
    if (auto* pi = dynamic_cast<LuxParser::PreIncrExprContext*>(expr)) {
        auto* inner = pi->expression();
        auto* type = resolveExprType(inner);
        if (type && !isInteger(type))
            error(expr, "operator '++' requires integer operand, got '" +
                             type->name + "'");
        if (!dynamic_cast<LuxParser::IdentExprContext*>(inner))
            error(expr, "operator '++' requires a variable (lvalue)");
        return type;
    }

    if (auto* pd = dynamic_cast<LuxParser::PreDecrExprContext*>(expr)) {
        auto* inner = pd->expression();
        auto* type = resolveExprType(inner);
        if (type && !isInteger(type))
            error(expr, "operator '--' requires integer operand, got '" +
                             type->name + "'");
        if (!dynamic_cast<LuxParser::IdentExprContext*>(inner))
            error(expr, "operator '--' requires a variable (lvalue)");
        return type;
    }

    // ── Post-increment/decrement ────────────────────────────────────
    if (auto* pi = dynamic_cast<LuxParser::PostIncrExprContext*>(expr)) {
        auto* inner = pi->expression();
        auto* type = resolveExprType(inner);
        if (type && !isInteger(type))
            error(expr, "operator '++' requires integer operand, got '" +
                             type->name + "'");
        if (!dynamic_cast<LuxParser::IdentExprContext*>(inner))
            error(expr, "operator '++' requires a variable (lvalue)");
        return type;
    }

    if (auto* pd = dynamic_cast<LuxParser::PostDecrExprContext*>(expr)) {
        auto* inner = pd->expression();
        auto* type = resolveExprType(inner);
        if (type && !isInteger(type))
            error(expr, "operator '--' requires integer operand, got '" +
                             type->name + "'");
        if (!dynamic_cast<LuxParser::IdentExprContext*>(inner))
            error(expr, "operator '--' requires a variable (lvalue)");
        return type;
    }

    // ── Ternary: cond ? trueExpr : falseExpr ────────────────────────
    if (auto* tern = dynamic_cast<LuxParser::TernaryExprContext*>(expr)) {
        auto exprs = tern->expression();
        resolveExprType(exprs[0]); // condition
        auto* trueType = resolveExprType(exprs[1]);
        auto* falseType = resolveExprType(exprs[2]);
        if (trueType && falseType && !isAssignable(trueType, falseType))
            error(expr, "ternary branches have incompatible types: '" +
                             trueType->name + "' and '" + falseType->name + "'");
        return trueType ? trueType : falseType;
    }

    // ── Is: expr is type ────────────────────────────────────────────
    if (auto* isE = dynamic_cast<LuxParser::IsExprContext*>(expr)) {
        resolveExprType(isE->expression());
        unsigned dims = 0;
        resolveTypeSpec(isE->typeSpec(), dims);
        return typeRegistry_.lookup("bool");
    }

    // ── Null coalescing: expr ?? default ────────────────────────────
    if (auto* nc = dynamic_cast<LuxParser::NullCoalExprContext*>(expr)) {
        auto exprs = nc->expression();
        auto* lhsType = resolveExprType(exprs[0]);
        auto* rhsType = resolveExprType(exprs[1]);
        if (lhsType && lhsType->kind != TypeKind::Pointer)
            error(expr, "left side of '\?\?' must be a pointer type, got '" +
                             lhsType->name + "'");
        if (lhsType && rhsType && lhsType->pointeeType &&
            !isAssignable(lhsType->pointeeType, rhsType))
            error(expr, "'\?\?' type mismatch: pointer to '" +
                             lhsType->pointeeType->name + "', default is '" +
                             rhsType->name + "'");
        return rhsType;
    }

    // ── Arrow access: expr->field ───────────────────────────────────
    if (auto* aa = dynamic_cast<LuxParser::ArrowAccessExprContext*>(expr)) {
        auto* baseType = resolveExprType(aa->expression());
        auto fieldName = aa->IDENTIFIER()->getText();

        if (baseType && baseType->kind != TypeKind::Pointer) {
            error(expr, "'->' requires a pointer type, got '" +
                             baseType->name + "'");
            return nullptr;
        }
        auto* pointee = baseType ? baseType->pointeeType : nullptr;
        if (pointee && pointee->kind != TypeKind::Struct) {
            error(expr, "'->' requires pointer to struct, got pointer to '" +
                             pointee->name + "'");
            return nullptr;
        }
        if (pointee) {
            for (auto& field : pointee->fields) {
                if (field.name == fieldName)
                    return field.typeInfo;
            }
            error(expr, "struct '" + pointee->name +
                             "' has no field '" + fieldName + "'");
        }
        return nullptr;
    }

    // ── Range: expr..expr ───────────────────────────────────────────
    if (auto* rng = dynamic_cast<LuxParser::RangeExprContext*>(expr)) {
        auto exprs = rng->expression();
        auto* startType = resolveExprType(exprs[0]);
        auto* endType   = resolveExprType(exprs[1]);
        if (startType && !isInteger(startType))
            error(expr, "range start must be integer, got '" +
                             startType->name + "'");
        if (endType && !isInteger(endType))
            error(expr, "range end must be integer, got '" +
                             endType->name + "'");
        return startType;
    }

    // ── Range inclusive: expr..=expr ─────────────────────────────────
    if (auto* rng = dynamic_cast<LuxParser::RangeInclExprContext*>(expr)) {
        auto exprs = rng->expression();
        auto* startType = resolveExprType(exprs[0]);
        auto* endType   = resolveExprType(exprs[1]);
        if (startType && !isInteger(startType))
            error(expr, "range start must be integer, got '" +
                             startType->name + "'");
        if (endType && !isInteger(endType))
            error(expr, "range end must be integer, got '" +
                             endType->name + "'");
        return startType;
    }

    // ── Spread: ...expr ─────────────────────────────────────────────
    if (auto* sp = dynamic_cast<LuxParser::SpreadExprContext*>(expr)) {
        return resolveExprType(sp->expression());
    }

    // ── Cast: expr as type ──────────────────────────────────────────
    if (auto* cast = dynamic_cast<LuxParser::CastExprContext*>(expr)) {
        auto* srcType = resolveExprType(cast->expression());
        unsigned dims = 0;
        auto* dstType = resolveTypeSpec(cast->typeSpec(), dims);

        if (srcType && dstType && srcType != dstType) {
            auto sk = srcType->kind;
            auto dk = dstType->kind;

            // string → pointer (e.g. string as cstring / string as *char)
            if (sk == TypeKind::String && dk == TypeKind::Pointer) {
                if (dstType->pointeeType &&
                    dstType->pointeeType->kind == TypeKind::Char) {
                    error(expr,
                        "cannot cast 'string' to 'cstring' directly. "
                        "Use 'cstr(expr)' to convert a string to a "
                        "null-terminated C string");
                } else {
                    error(expr,
                        "cannot cast 'string' to '" + dstType->name +
                        "': incompatible types");
                }
            }
            // pointer → string
            else if (sk == TypeKind::Pointer && dk == TypeKind::String) {
                if (srcType->pointeeType &&
                    srcType->pointeeType->kind == TypeKind::Char) {
                    error(expr,
                        "cannot cast 'cstring' to 'string' directly. "
                        "Use 'fromCStr(expr)' to convert a C string to "
                        "a TM string");
                } else {
                    error(expr,
                        "cannot cast '" + srcType->name +
                        "' to 'string': incompatible types");
                }
            }
            // struct/extended/string → numeric or vice-versa
            else if ((sk == TypeKind::Struct || sk == TypeKind::Extended ||
                      sk == TypeKind::String) &&
                     (dk == TypeKind::Integer || dk == TypeKind::Float ||
                      dk == TypeKind::Bool)) {
                error(expr,
                    "cannot cast '" + srcType->name + "' to '" +
                    dstType->name + "': incompatible types");
            }
            else if ((sk == TypeKind::Integer || sk == TypeKind::Float ||
                      sk == TypeKind::Bool) &&
                     (dk == TypeKind::Struct || dk == TypeKind::Extended ||
                      dk == TypeKind::String)) {
                error(expr,
                    "cannot cast '" + srcType->name + "' to '" +
                    dstType->name + "': incompatible types");
            }
        }

        return dstType;
    }

    // ── Sizeof: sizeof(type) ────────────────────────────────────────
    if (auto* sz = dynamic_cast<LuxParser::SizeofExprContext*>(expr)) {
        unsigned dims = 0;
        auto* ti = resolveTypeSpec(sz->typeSpec(), dims);
        if (!ti) error(expr, "sizeof: unknown type");
        return typeRegistry_.lookup("int64");
    }

    // ── Typeof: typeof(expr) ────────────────────────────────────────
    if (auto* to = dynamic_cast<LuxParser::TypeofExprContext*>(expr)) {
        resolveExprType(to->expression());
        return typeRegistry_.lookup("string");
    }

    // ── Method call: expr.method(args) ─────────────────────────────
    if (auto* mc = dynamic_cast<LuxParser::MethodCallExprContext*>(expr)) {
        auto* receiverType = resolveExprType(mc->expression());
        auto methodName = mc->IDENTIFIER()->getText();

        std::vector<const TypeInfo*> argTypes;
        if (auto* argList = mc->argList()) {
            for (auto* argExpr : argList->expression()) {
                argTypes.push_back(resolveExprType(argExpr));
            }
        }

        if (!receiverType) return nullptr;

        unsigned recvArrayDims = resolveExprArrayDims(mc->expression());

        const MethodDescriptor* desc = nullptr;

        if (recvArrayDims > 0) {
            // Array method
            desc = methodRegistry_.lookupArrayMethod(methodName);
            if (!desc) {
                error(expr, "array type '[]" + receiverType->name +
                            "' has no method '" + methodName + "'");
                return nullptr;
            }
            // Validate numeric-only constraint
            if (desc->requireNumeric && !isNumeric(receiverType)) {
                error(expr, "method '" + methodName +
                            "' requires numeric element type, got '" +
                            receiverType->name + "'");
                return nullptr;
            }
        } else if (receiverType->kind == TypeKind::Struct) {
            // Struct field function call: obj.callback(args)
            for (auto& field : receiverType->fields) {
                if (field.name == methodName && field.typeInfo &&
                    field.typeInfo->kind == TypeKind::Function) {
                    if (argTypes.size() != field.typeInfo->paramTypes.size()) {
                        error(expr, "function call expects " +
                            std::to_string(field.typeInfo->paramTypes.size()) +
                            " arguments, got " + std::to_string(argTypes.size()));
                    }
                    return field.typeInfo->returnType;
                }
            }
            // Struct method via `extend` block
            auto smIt = structMethods_.find(receiverType->name);
            if (smIt != structMethods_.end()) {
                for (auto& sm : smIt->second) {
                    if (sm.name == methodName && !sm.isStatic) {
                        if (argTypes.size() != sm.paramTypes.size()) {
                            error(expr, "method '" + methodName + "' expects " +
                                std::to_string(sm.paramTypes.size()) +
                                " arguments, got " + std::to_string(argTypes.size()));
                        }
                        return sm.returnType;
                    }
                }
            }
            error(expr, "struct '" + receiverType->name +
                        "' has no method '" + methodName + "'");
            return nullptr;
        } else if (receiverType->kind == TypeKind::Extended) {
            // Extended type method (e.g. vec.push, vec.pop)
            auto* extDesc = extTypeRegistry_.lookup(receiverType->extendedKind);
            if (!extDesc) {
                error(expr, "unknown extended type '" + receiverType->extendedKind + "'");
                return nullptr;
            }
            for (auto& md : extDesc->methods) {
                if (md.name == methodName) {
                    desc = &md;
                    break;
                }
            }
            if (!desc) {
                error(expr, "type '" + receiverType->name +
                            "' has no method '" + methodName + "'");
                return nullptr;
            }
            if (desc->requireNumeric) {
                auto* elemTI = receiverType->elementType;
                if (elemTI && !isNumeric(elemTI)) {
                    error(expr, "method '" + methodName +
                                "' requires numeric element type, got '" +
                                elemTI->name + "'");
                    return nullptr;
                }
            }
        } else {
            // Built-in type method
            desc = methodRegistry_.lookup(receiverType->kind, methodName);
            if (!desc) {
                error(expr, "type '" + receiverType->name +
                            "' has no method '" + methodName + "'");
                return nullptr;
            }
            // Validate signed/unsigned constraints
            if (desc->requireSigned && receiverType->kind == TypeKind::Integer &&
                !receiverType->isSigned) {
                error(expr, "method '" + methodName +
                            "' requires a signed integer type");
                return nullptr;
            }
            if (desc->requireUnsigned && receiverType->kind == TypeKind::Integer &&
                receiverType->isSigned) {
                error(expr, "method '" + methodName +
                            "' requires an unsigned integer type");
                return nullptr;
            }
        }

        // Validate argument count
        if (argTypes.size() != desc->paramTypes.size()) {
            error(expr, "method '" + methodName + "' expects " +
                std::to_string(desc->paramTypes.size()) +
                " arguments, got " + std::to_string(argTypes.size()));
            return nullptr;
        }

        // Validate argument types
        for (size_t i = 0; i < argTypes.size(); i++) {
            if (!argTypes[i]) continue;
            const TypeInfo* expectedParam = nullptr;
            if (desc->paramTypes[i] == "_self")
                expectedParam = receiverType;
            else if (desc->paramTypes[i] == "_elem") {
                if (receiverType->kind == TypeKind::Extended && receiverType->elementType)
                    expectedParam = receiverType->elementType;
                else
                    expectedParam = receiverType;
            } else if (desc->paramTypes[i] == "_key") {
                if (receiverType->kind == TypeKind::Extended && receiverType->keyType)
                    expectedParam = receiverType->keyType;
            } else if (desc->paramTypes[i] == "_val") {
                if (receiverType->kind == TypeKind::Extended && receiverType->valueType)
                    expectedParam = receiverType->valueType;
            } else
                expectedParam = typeRegistry_.lookup(desc->paramTypes[i]);

            if (expectedParam && !isAssignable(expectedParam, argTypes[i])) {
                error(expr, "method '" + methodName + "' argument " +
                    std::to_string(i + 1) + " type mismatch: expected '" +
                    expectedParam->name + "', got '" + argTypes[i]->name + "'");
            }
        }

        // Resolve return type
        if (desc->returnType == "_self")
            return receiverType;
        if (desc->returnType == "_elem") {
            if (receiverType->kind == TypeKind::Extended && receiverType->elementType)
                return receiverType->elementType;
            return receiverType;
        }
        if (desc->returnType == "_key") {
            if (receiverType->kind == TypeKind::Extended && receiverType->keyType)
                return receiverType->keyType;
            return receiverType;
        }
        if (desc->returnType == "_val") {
            if (receiverType->kind == TypeKind::Extended && receiverType->valueType)
                return receiverType->valueType;
            return receiverType;
        }
        return typeRegistry_.lookup(desc->returnType);
    }

    // ── Function call: expr(args) ───────────────────────────────────
    if (auto* call = dynamic_cast<LuxParser::FnCallExprContext*>(expr)) {
        auto* callee = call->expression();
        auto* calleeType = resolveExprType(callee);

        // Detect callee name for polymorphic builtin detection
        std::string calleeName;
        if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(callee))
            calleeName = ident->IDENTIFIER()->getText();

        std::vector<const TypeInfo*> argTypes;
        if (auto* argList = call->argList()) {
            for (auto* argExpr : argList->expression()) {
                argTypes.push_back(resolveExprType(argExpr));
            }
        }

        // User-defined function with full type info
        if (calleeType && calleeType->kind == TypeKind::Function) {
            size_t paramCount = calleeType->paramTypes.size();

            if (calleeType->isVariadic) {
                size_t minArgs = paramCount > 0 ? paramCount - 1 : 0;
                if (argTypes.size() < minArgs) {
                    error(expr, "function call expects at least " +
                                     std::to_string(minArgs) +
                                     " arguments, got " +
                                     std::to_string(argTypes.size()));
                }
                // Validate fixed parameter types
                for (size_t i = 0; i < std::min(argTypes.size(), paramCount); i++) {
                    if (argTypes[i] && calleeType->paramTypes[i] &&
                        !isAssignable(calleeType->paramTypes[i], argTypes[i])) {
                        error(expr,
                            "argument " + std::to_string(i + 1) +
                            " type mismatch: expected '" +
                            calleeType->paramTypes[i]->name + "', got '" +
                            argTypes[i]->name + "'");
                    }
                }
            } else {
                if (argTypes.size() != paramCount) {
                    error(expr, "function call expects " +
                                     std::to_string(paramCount) +
                                     " arguments, got " +
                                     std::to_string(argTypes.size()));
                } else if (calleeName != "toString") {
                    for (size_t i = 0; i < argTypes.size(); i++) {
                        if (argTypes[i] && calleeType->paramTypes[i] &&
                            !isAssignable(calleeType->paramTypes[i], argTypes[i])) {
                            error(expr,
                                "argument " + std::to_string(i + 1) +
                                " type mismatch: expected '" +
                                calleeType->paramTypes[i]->name + "', got '" +
                                argTypes[i]->name + "'");
                        }
                    }
                }
            }
            return calleeType->returnType;
        }

        // Builtin function via registry
        if (!calleeName.empty()) {
            auto* sig = builtinRegistry_.lookup(calleeName);
            if (sig) {
                // Validate argument count
                if (sig->isVariadic) {
                    if (argTypes.size() < sig->paramTypes.size()) {
                        error(expr, "'" + calleeName + "' expects at least " +
                            std::to_string(sig->paramTypes.size()) +
                            " argument(s), got " +
                            std::to_string(argTypes.size()));
                    }
                } else if (argTypes.size() != sig->paramTypes.size()) {
                    error(expr, "'" + calleeName + "' expects " +
                        std::to_string(sig->paramTypes.size()) +
                        " argument(s), got " +
                        std::to_string(argTypes.size()));
                } else if (!sig->isPolymorphic) {
                    // Validate argument types
                    for (size_t i = 0; i < argTypes.size(); i++) {
                        if (!argTypes[i]) continue;
                        auto& expected = sig->paramTypes[i];
                        if (expected == "_any" || expected == "_numeric" ||
                            expected == "_integer" || expected == "_float")
                            continue;
                        auto* expectedTI = resolveBuiltinReturnType(expected);
                        if (expectedTI && !isAssignable(expectedTI, argTypes[i])) {
                            error(expr, "'" + calleeName + "' argument " +
                                std::to_string(i + 1) + ": expected '" +
                                expected + "', got '" + argTypes[i]->name + "'");
                        }
                    }
                }

                // Resolve return type
                auto& retName = sig->returnType;
                if (retName == "_any" || retName == "_numeric") {
                    // Polymorphic: return type matches first argument
                    if (!argTypes.empty() && argTypes[0])
                        return argTypes[0];
                    return typeRegistry_.lookup("int32");
                }
                return resolveBuiltinReturnType(retName);
            }
        }

        return nullptr;
    }

    // ── Field access: expr.field ─────────────────────────────────────
    if (auto* fa = dynamic_cast<LuxParser::FieldAccessExprContext*>(expr)) {
        auto* baseType = resolveExprType(fa->expression());
        auto fieldName = fa->IDENTIFIER()->getText();

        // .len on array/variadic params → int64
        if (fieldName == "len") {
            if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(fa->expression())) {
                auto it = locals_.find(ident->IDENTIFIER()->getText());
                if (it != locals_.end() && it->second.arrayDims > 0)
                    return typeRegistry_.lookup("int64");
            }
        }

        if (baseType && (baseType->kind == TypeKind::Struct || baseType->kind == TypeKind::Union)) {
            for (auto& field : baseType->fields) {
                if (field.name == fieldName)
                    return field.typeInfo;
            }
            error(expr, "'" + baseType->name +
                             "' has no field '" + fieldName + "'");
        }
        return nullptr;
    }

    // ── Index: expr[index] ──────────────────────────────────────────
    if (auto* idx = dynamic_cast<LuxParser::IndexExprContext*>(expr)) {
        auto exprs = idx->expression();
        auto* baseType = resolveExprType(exprs[0]);
        auto* indexType = resolveExprType(exprs[1]);

        // Map<K,V>[key] returns V — key type must match K
        if (baseType && baseType->kind == TypeKind::Extended && baseType->keyType) {
            if (indexType && !isAssignable(baseType->keyType, indexType))
                error(expr, "map key type mismatch: expected '" +
                             baseType->keyType->name + "', got '" +
                             indexType->name + "'");
            return baseType->valueType;
        }

        // Vec<T>[i] and array[i] — index must be integer
        if (indexType && !isInteger(indexType))
            error(expr, "index must be integer, got '" +
                             indexType->name + "'");

        // Vec<T>[i] returns T
        if (baseType && baseType->kind == TypeKind::Extended && baseType->elementType)
            return baseType->elementType;

        return baseType;
    }

    // ── Struct / Union literal: Name { field: expr, ... } ────────────
    if (auto* sl = dynamic_cast<LuxParser::StructLitExprContext*>(expr)) {
        auto ids = sl->IDENTIFIER();
        if (ids.empty()) return nullptr;

        auto typeName = ids[0]->getText();
        auto* typeInfo = typeRegistry_.lookup(typeName);

        if (!typeInfo) {
            error(expr, "unknown type '" + typeName + "'");
            return nullptr;
        }
        if (typeInfo->kind != TypeKind::Struct && typeInfo->kind != TypeKind::Union) {
            error(expr, "'" + typeName + "' is not a struct or union type");
            return nullptr;
        }

        auto fieldExprs = sl->expression();
        size_t fieldCount = ids.size() - 1;

        if (typeInfo->kind == TypeKind::Union) {
            if (fieldCount != 1) {
                error(expr, "union '" + typeName +
                                 "' literal must initialize exactly 1 field, got " +
                                 std::to_string(fieldCount));
            }
        } else {
            if (fieldCount != typeInfo->fields.size()) {
                error(expr, "struct '" + typeName + "' has " +
                                 std::to_string(typeInfo->fields.size()) +
                                 " fields, got " + std::to_string(fieldCount));
            }
        }

        for (size_t i = 0; i < fieldCount; i++) {
            auto fieldName = ids[i + 1]->getText();
            bool found = false;
            for (auto& sf : typeInfo->fields) {
                if (sf.name == fieldName) {
                    found = true;
                    if (i < fieldExprs.size()) {
                        auto* valType = resolveExprType(fieldExprs[i]);
                        if (valType && sf.typeInfo &&
                            !isAssignable(sf.typeInfo, valType)) {
                            error(expr, "field '" + fieldName +
                                             "' expects type '" +
                                             sf.typeInfo->name + "', got '" +
                                             valType->name + "'");
                        }
                    }
                    break;
                }
            }
            if (!found)
                error(expr, "'" + typeName +
                                 "' has no field '" + fieldName + "'");
        }
        return typeInfo;
    }

    // ── Static method call: Struct::method(args) ─────────────────────
    if (auto* smc = dynamic_cast<LuxParser::StaticMethodCallExprContext*>(expr)) {
        auto ids = smc->IDENTIFIER();
        auto structName = ids[0]->getText();
        auto methodName = ids[1]->getText();

        auto* structType = typeRegistry_.lookup(structName);
        if (!structType) {
            error(expr, "unknown type '" + structName + "'");
            return nullptr;
        }
        if (structType->kind != TypeKind::Struct) {
            error(expr, "'" + structName + "' is not a struct type");
            return nullptr;
        }

        auto smIt = structMethods_.find(structName);
        if (smIt == structMethods_.end()) {
            error(expr, "struct '" + structName + "' has no methods");
            return nullptr;
        }

        for (auto& sm : smIt->second) {
            if (sm.name == methodName && sm.isStatic) {
                std::vector<const TypeInfo*> argTypes;
                if (auto* argList = smc->argList()) {
                    for (auto* argExpr : argList->expression())
                        argTypes.push_back(resolveExprType(argExpr));
                }
                if (argTypes.size() != sm.paramTypes.size()) {
                    error(expr, "static method '" + structName + "::" + methodName +
                        "' expects " + std::to_string(sm.paramTypes.size()) +
                        " arguments, got " + std::to_string(argTypes.size()));
                }
                return sm.returnType;
            }
        }

        error(expr, "struct '" + structName +
                    "' has no static method '" + methodName + "'");
        return nullptr;
    }

    // ── Enum access: Enum::Variant ──────────────────────────────────
    if (auto* ea = dynamic_cast<LuxParser::EnumAccessExprContext*>(expr)) {
        auto ids = ea->IDENTIFIER();
        auto enumName = ids[0]->getText();
        auto variant = ids[1]->getText();

        auto* enumType = typeRegistry_.lookup(enumName);
        if (!enumType) {
            error(expr, "unknown enum type '" + enumName + "'");
            return nullptr;
        }
        if (enumType->kind != TypeKind::Enum) {
            error(expr, "'" + enumName + "' is not an enum type");
            return nullptr;
        }

        bool found = false;
        for (auto& v : enumType->enumVariants) {
            if (v == variant) { found = true; break; }
        }
        if (!found)
            error(expr, "enum '" + enumName +
                             "' has no variant '" + variant + "'");
        return enumType;
    }

    // ── Address-of: &var ────────────────────────────────────────────
    if (auto* addr = dynamic_cast<LuxParser::AddrOfExprContext*>(expr)) {
        auto varName = addr->IDENTIFIER()->getText();
        auto it = locals_.find(varName);
        if (it == locals_.end()) {
            error(expr, "undefined variable '" + varName + "'");
            return nullptr;
        }
        return getPointerType(it->second.type);
    }

    // ── Dereference: *expr ──────────────────────────────────────────
    if (auto* deref = dynamic_cast<LuxParser::DerefExprContext*>(expr)) {
        auto* operand = resolveExprType(deref->expression());
        if (operand && operand->kind != TypeKind::Pointer) {
            error(expr, "cannot dereference non-pointer type '" +
                             operand->name + "'");
            return nullptr;
        }
        if (operand && operand->pointeeType)
            return operand->pointeeType;
        return nullptr;
    }

    // ── Array literal: [expr, expr, ...] ────────────────────────────
    if (auto* arr = dynamic_cast<LuxParser::ArrayLitExprContext*>(expr)) {
        auto elements = arr->expression();
        if (elements.empty()) return nullptr;

        auto* firstType = resolveExprType(elements[0]);
        for (size_t i = 1; i < elements.size(); i++) {
            auto* elemType = resolveExprType(elements[i]);
            if (firstType && elemType && !isAssignable(firstType, elemType))
                error(expr, "array element type mismatch: expected '" +
                                 firstType->name + "', got '" +
                                 elemType->name + "'");
        }
        return firstType;
    }

    // ── List comprehension: [expr | for type x in iterable if cond] ─
    if (auto* lc = dynamic_cast<LuxParser::ListCompExprContext*>(expr)) {
        // Register the loop variable temporarily
        unsigned varDims = 0;
        auto* varType = resolveTypeSpec(lc->typeSpec(), varDims);
        auto  varName = lc->IDENTIFIER()->getText();

        auto prev = locals_.find(varName);
        auto hadPrev = prev != locals_.end();
        VarInfo prevInfo;
        if (hadPrev) prevInfo = prev->second;

        if (varType)
            locals_[varName] = { varType, varDims };

        // Resolve iterable expression
        resolveExprType(lc->expression(1));

        // Resolve optional filter
        auto exprs = lc->expression();
        if (exprs.size() > 2)
            resolveExprType(exprs[2]);

        // Resolve the value expression — its type is the element type
        auto* elemType = resolveExprType(lc->expression(0));

        // Restore scope
        if (hadPrev)
            locals_[varName] = prevInfo;
        else
            locals_.erase(varName);

        return elemType;
    }

    return nullptr;
}

// ═══════════════════════════════════════════════════════════════════════
//  Top-level declaration checks
// ═══════════════════════════════════════════════════════════════════════

void Checker::setNamespaceContext(const NamespaceRegistry* registry,
                                   const std::string& currentNamespace,
                                   const std::string& currentFile) {
    nsRegistry_       = registry;
    currentNamespace_ = currentNamespace;
    currentFile_      = currentFile;
}

void Checker::setCBindings(const CBindings* bindings) {
    cBindings_ = bindings;
}

bool Checker::isKnownFunction(const std::string& name) const {
    // 1. Local file function
    if (functions_.count(name)) return true;
    // 2. Builtin
    if (globalBuiltins_.count(name)) return true;
    // 3. Std import
    if (imports_.isImported(name)) return true;
    // 4. User import
    if (userImports_.count(name)) return true;
    // 5. Same-namespace symbol from another file
    if (nsRegistry_ && !currentNamespace_.empty()) {
        auto* sym = nsRegistry_->findSymbol(currentNamespace_, name);
        if (sym && sym->kind == ExportedSymbol::Function &&
            sym->sourceFile != currentFile_)
            return true;
    }
    return false;
}

bool Checker::isKnownType(const std::string& name) const {
    if (typeRegistry_.lookup(name)) return true;
    // Check user imports for struct/enum/alias
    if (userImports_.count(name)) return true;
    // Check same-namespace types from other files
    if (nsRegistry_ && !currentNamespace_.empty()) {
        auto* sym = nsRegistry_->findSymbol(currentNamespace_, name);
        if (sym && (sym->kind == ExportedSymbol::Struct ||
                    sym->kind == ExportedSymbol::Enum ||
                    sym->kind == ExportedSymbol::TypeAlias) &&
            sym->sourceFile != currentFile_)
            return true;
    }
    return false;
}

void Checker::checkUseDecls(LuxParser::ProgramContext* tree) {
    for (auto* useDecl : tree->useDecl()) {
        if (auto* item = dynamic_cast<LuxParser::UseItemContext*>(useDecl)) {
            std::string path;
            for (auto* id : item->modulePath()->IDENTIFIER()) {
                if (!path.empty()) path += "::";
                path += id->getText();
            }
            auto symbolName = item->IDENTIFIER()->getText();

            if (NamespaceRegistry::isStdModule(path)) {
                imports_.addImport(path, symbolName);
            } else if (nsRegistry_ && nsRegistry_->hasNamespace(path)) {
                auto* sym = nsRegistry_->findSymbol(path, symbolName);
                if (!sym) {
                    error(item, "namespace '" + path +
                                "' does not export '" + symbolName + "'");
                } else {
                    userImports_[symbolName] = path;
                }
            } else {
                error(item, "unknown module or namespace '" + path + "'");
            }
        } else if (auto* grp = dynamic_cast<LuxParser::UseGroupContext*>(useDecl)) {
            std::string path;
            for (auto* id : grp->modulePath()->IDENTIFIER()) {
                if (!path.empty()) path += "::";
                path += id->getText();
            }

            if (NamespaceRegistry::isStdModule(path)) {
                for (auto* id : grp->IDENTIFIER()) {
                    imports_.addImport(path, id->getText());
                }
            } else if (nsRegistry_ && nsRegistry_->hasNamespace(path)) {
                for (auto* id : grp->IDENTIFIER()) {
                    auto symbolName = id->getText();
                    auto* sym = nsRegistry_->findSymbol(path, symbolName);
                    if (!sym) {
                        error(grp, "namespace '" + path +
                                    "' does not export '" + symbolName + "'");
                    } else {
                        userImports_[symbolName] = path;
                    }
                }
            } else {
                error(grp, "unknown module or namespace '" + path + "'");
            }
        }
    }
}

void Checker::checkTypeAliasDecl(LuxParser::TypeAliasDeclContext* decl) {
    auto name = decl->IDENTIFIER()->getText();

    if (typeRegistry_.lookup(name)) {
        error(decl, "type '" + name + "' already defined");
        return;
    }

    // For now, we register a Function TypeInfo for fn(...) -> T aliases
    auto* typeSpecCtx = decl->typeSpec();
    if (auto* fnSpec = typeSpecCtx->fnTypeSpec()) {
        TypeInfo ti;
        ti.name = name;
        ti.kind = TypeKind::Function;
        ti.bitWidth = 0;
        ti.isSigned = false;
        ti.builtinSuffix = "ptr";

        // Resolve return type
        auto retSpecs = fnSpec->typeSpec();
        auto retName = resolveBaseTypeName(retSpecs.back());
        auto* retTI = typeRegistry_.lookup(retName);
        if (!retTI) {
            error(decl, "unknown return type '" + retName +
                             "' in type alias '" + name + "'");
            return;
        }
        ti.returnType = retTI;

        // Resolve parameter types (all typeSpecs except the last one which is return)
        for (size_t i = 0; i + 1 < retSpecs.size(); i++) {
            auto paramName = resolveBaseTypeName(retSpecs[i]);
            auto* paramTI = typeRegistry_.lookup(paramName);
            if (!paramTI) {
                error(decl, "unknown param type '" + paramName +
                                 "' in type alias '" + name + "'");
                return;
            }
            ti.paramTypes.push_back(paramTI);
        }

        typeRegistry_.registerType(std::move(ti));
    } else {
        // Simple alias: type MyInt = int32;
        auto baseName = resolveBaseTypeName(typeSpecCtx);
        auto* baseTI = typeRegistry_.lookup(baseName);
        if (!baseTI) {
            error(decl, "unknown type '" + baseName +
                             "' in type alias '" + name + "'");
            return;
        }
        // Register as a copy with the alias name
        TypeInfo ti = *baseTI;
        ti.name = name;
        typeRegistry_.registerType(std::move(ti));
    }
}

void Checker::checkStructDecl(LuxParser::StructDeclContext* decl) {
    auto name = decl->IDENTIFIER()->getText();

    if (typeRegistry_.lookup(name)) {
        error(decl, "type '" + name + "' already defined");
        return;
    }

    // Register skeleton first so self-referencing pointer fields
    // (e.g. *Node inside Node) can resolve the type.
    TypeInfo skeleton;
    skeleton.name = name;
    skeleton.kind = TypeKind::Struct;
    skeleton.bitWidth = 0;
    skeleton.isSigned = false;
    typeRegistry_.registerType(skeleton);

    TypeInfo ti = skeleton;
    std::unordered_set<std::string> seen;
    for (auto* field : decl->structField()) {
        unsigned fieldDims = 0;
        auto* fieldTI = resolveTypeSpec(field->typeSpec(), fieldDims);
        if (!fieldTI) {
            error(field, "unknown field type in struct '" + name + "'");
            return;
        }
        auto fieldName = field->IDENTIFIER()->getText();
        if (!seen.insert(fieldName).second) {
            error(field, "duplicate field '" + fieldName +
                             "' in struct '" + name + "'");
            return;
        }
        ti.fields.push_back({ fieldName, fieldTI });
    }

    typeRegistry_.registerType(std::move(ti));
}

void Checker::checkUnionDecl(LuxParser::UnionDeclContext* decl) {
    auto name = decl->IDENTIFIER()->getText();

    if (typeRegistry_.lookup(name)) {
        error(decl, "type '" + name + "' already defined");
        return;
    }

    TypeInfo ti;
    ti.name = name;
    ti.kind = TypeKind::Union;
    ti.bitWidth = 0;
    ti.isSigned = false;

    std::unordered_set<std::string> seen;
    for (auto* field : decl->unionField()) {
        unsigned fieldDims = 0;
        auto* fieldTI = resolveTypeSpec(field->typeSpec(), fieldDims);
        if (!fieldTI) {
            error(field, "unknown field type in union '" + name + "'");
            return;
        }
        auto fieldName = field->IDENTIFIER()->getText();
        if (!seen.insert(fieldName).second) {
            error(field, "duplicate field '" + fieldName +
                             "' in union '" + name + "'");
            return;
        }
        ti.fields.push_back({ fieldName, fieldTI });
    }

    typeRegistry_.registerType(std::move(ti));
}

void Checker::checkEnumDecl(LuxParser::EnumDeclContext* decl) {
    auto identifiers = decl->IDENTIFIER();
    auto name = identifiers[0]->getText();

    if (typeRegistry_.lookup(name)) {
        error(decl, "type '" + name + "' already defined");
        return;
    }

    TypeInfo ti;
    ti.name = name;
    ti.kind = TypeKind::Enum;
    ti.bitWidth = 32;
    ti.isSigned = false;
    ti.builtinSuffix = "i32";

    std::unordered_set<std::string> seen;
    for (size_t i = 1; i < identifiers.size(); i++) {
        auto variant = identifiers[i]->getText();
        if (!seen.insert(variant).second) {
            error(decl, "duplicate enum variant '" + variant +
                             "' in enum '" + name + "'");
            return;
        }
        ti.enumVariants.push_back(variant);
    }

    typeRegistry_.registerType(std::move(ti));
}

void Checker::checkExtendDecl(LuxParser::ExtendDeclContext* decl) {
    auto structName = decl->IDENTIFIER()->getText();
    auto* structTI = typeRegistry_.lookup(structName);
    if (!structTI) {
        error(decl, "cannot extend unknown type '" + structName + "'");
        return;
    }
    if (structTI->kind != TypeKind::Struct) {
        error(decl, "'" + structName + "' is not a struct type");
        return;
    }

    for (auto* method : decl->extendMethod()) {
        auto methodName = method->IDENTIFIER(0)->getText();

        unsigned retDims = 0;
        auto* retType = resolveTypeSpec(method->typeSpec(), retDims);
        if (!retType) continue;

        StructMethodInfo info;
        info.name = methodName;
        info.returnType = retType;
        info.isStatic = (method->AMPERSAND() == nullptr);

        // Static methods use paramList, instance methods use direct param()
        std::vector<LuxParser::ParamContext*> params;
        if (info.isStatic) {
            if (auto* pl = method->paramList())
                params = pl->param();
        } else {
            params = method->param();
        }

        for (auto* param : params) {
            unsigned pDims = 0;
            auto* pType = resolveTypeSpec(param->typeSpec(), pDims);
            if (!pType) continue;
            info.paramTypes.push_back(pType);
        }

        structMethods_[structName].push_back(std::move(info));
    }
}

void Checker::registerFunctionSignature(LuxParser::FunctionDeclContext* func) {
    auto funcName = func->IDENTIFIER()->getText();

    unsigned retDims = 0;
    auto* retType = resolveTypeSpec(func->typeSpec(), retDims);
    if (!retType) return;

    bool isVariadic = false;
    std::vector<const TypeInfo*> paramTypes;
    if (auto* paramList = func->paramList()) {
        auto params = paramList->param();
        for (size_t i = 0; i < params.size(); i++) {
            auto* param = params[i];
            unsigned pDims = 0;
            auto* pType = resolveTypeSpec(param->typeSpec(), pDims);
            if (!pType) return;
            paramTypes.push_back(pType);

            if (param->SPREAD()) {
                if (i != params.size() - 1) {
                    error(param, "variadic parameter must be the last parameter");
                    return;
                }
                isVariadic = true;
            }
        }
    }

    auto* funcType = makeFunctionType(retType, paramTypes, isVariadic);
    functions_[funcName] = funcType;
}

// ═══════════════════════════════════════════════════════════════════════
//  Function body check
// ═══════════════════════════════════════════════════════════════════════

void Checker::checkFunction(LuxParser::FunctionDeclContext* func) {
    unsigned retDims = 0;
    auto* retType = resolveTypeSpec(func->typeSpec(), retDims);
    if (!retType) return;

    // Register parameters as locals
    if (auto* paramList = func->paramList()) {
        for (auto* param : paramList->param()) {
            auto paramName = param->IDENTIFIER()->getText();
            unsigned pDims = 0;
            auto* pType = resolveTypeSpec(param->typeSpec(), pDims);
            if (!pType) continue;

            if (locals_.count(paramName)) {
                error(param, "duplicate parameter name '" + paramName + "'");
                continue;
            }

            // Variadic param is treated as array inside the function
            if (param->SPREAD())
                locals_[paramName] = {pType, 1};
            else
                locals_[paramName] = {pType, pDims};
        }
    }

    checkBlock(func->block(), retType);
}

void Checker::checkBlock(LuxParser::BlockContext* block,
                         const TypeInfo* retType) {
    for (auto* stmt : block->statement()) {
        if (auto* varDecl = stmt->varDeclStmt()) {
            checkVarDeclStmt(varDecl);
        } else if (auto* assign = stmt->assignStmt()) {
            checkAssignStmt(assign);
        } else if (auto* compound = stmt->compoundAssignStmt()) {
            checkCompoundAssignStmt(compound);
        } else if (auto* fieldAssign = stmt->fieldAssignStmt()) {
            checkFieldAssignStmt(fieldAssign);
        } else if (stmt->arrowAssignStmt()) {
            // Arrow assign: ptr->field = val (basic validation via expression)
            resolveExprType(stmt->arrowAssignStmt()->expression());
        } else if (stmt->arrowCompoundAssignStmt()) {
            // Arrow compound: ptr->field += val
            resolveExprType(stmt->arrowCompoundAssignStmt()->expression());
        } else if (auto* derefAssign = stmt->derefAssignStmt()) {
            checkDerefAssignStmt(derefAssign);
        } else if (auto* call = stmt->callStmt()) {
            checkCallStmt(call);
        } else if (auto* exprS = stmt->exprStmt()) {
            checkExprStmt(exprS);
        } else if (auto* ret = stmt->returnStmt()) {
            checkReturnStmt(ret, retType);
        } else if (auto* ifS = stmt->ifStmt()) {
            checkIfStmt(ifS, retType);
        } else if (auto* forS = stmt->forStmt()) {
            if (auto* forIn = dynamic_cast<LuxParser::ForInStmtContext*>(forS))
                checkForInStmt(forIn, retType);
            else if (auto* forC = dynamic_cast<LuxParser::ForClassicStmtContext*>(forS))
                checkForClassicStmt(forC, retType);
        } else if (stmt->loopStmt()) {
            loopDepth_++;
            checkBlock(stmt->loopStmt()->block(), retType);
            loopDepth_--;
        } else if (auto* ws = stmt->whileStmt()) {
            resolveExprType(ws->expression());
            loopDepth_++;
            checkBlock(ws->block(), retType);
            loopDepth_--;
        } else if (auto* dw = stmt->doWhileStmt()) {
            loopDepth_++;
            checkBlock(dw->block(), retType);
            loopDepth_--;
            resolveExprType(dw->expression());
        } else if (stmt->breakStmt()) {
            if (loopDepth_ == 0)
                error(stmt, "'break' used outside of a loop");
        } else if (stmt->continueStmt()) {
            if (loopDepth_ == 0)
                error(stmt, "'continue' used outside of a loop");
        } else if (auto* sw = stmt->switchStmt()) {
            checkSwitchStmt(sw, retType);
        } else if (auto* lk = stmt->lockStmt()) {
            resolveExprType(lk->expression());
            checkBlock(lk->block(), retType);
        } else if (auto* def = stmt->deferStmt()) {
            if (def->callStmt())
                checkCallStmt(def->callStmt());
            else if (def->exprStmt())
                checkExprStmt(def->exprStmt());
        }
    }
}

void Checker::checkSwitchStmt(LuxParser::SwitchStmtContext* stmt,
                              const TypeInfo* retType) {
    auto* exprType = resolveExprType(stmt->expression());
    if (exprType && !isInteger(exprType) && exprType->name != "char") {
        error(stmt, "switch expression must be an integer or char type, got '" +
              exprType->name + "'");
    }
    for (auto* cc : stmt->caseClause()) {
        for (auto* caseExpr : cc->expression()) {
            resolveExprType(caseExpr);
        }
        checkBlock(cc->block(), retType);
    }
    if (auto* dc = stmt->defaultClause()) {
        checkBlock(dc->block(), retType);
    }
}

void Checker::checkIfStmt(LuxParser::IfStmtContext* stmt,
                           const TypeInfo* retType) {
    // Check the if condition
    resolveExprType(stmt->expression());

    // Check the if body
    checkBlock(stmt->block(), retType);

    // Check else-if clauses
    for (auto* elseIf : stmt->elseIfClause()) {
        resolveExprType(elseIf->expression());
        checkBlock(elseIf->block(), retType);
    }

    // Check else clause
    if (auto* elseC = stmt->elseClause()) {
        checkBlock(elseC->block(), retType);
    }
}

void Checker::checkForInStmt(LuxParser::ForInStmtContext* stmt,
                              const TypeInfo* retType) {
    unsigned dims = 0;
    auto* iterType = resolveTypeSpec(stmt->typeSpec(), dims);
    auto iterName = stmt->IDENTIFIER()->getText();

    auto* iterableType = resolveExprType(stmt->expression());

    // Register the loop variable
    if (iterType)
        locals_[iterName] = {iterType, dims};

    loopDepth_++;
    checkBlock(stmt->block(), retType);
    loopDepth_--;

    locals_.erase(iterName);
}

void Checker::checkForClassicStmt(LuxParser::ForClassicStmtContext* stmt,
                                   const TypeInfo* retType) {
    unsigned dims = 0;
    auto* varType = resolveTypeSpec(stmt->typeSpec(), dims);
    auto varName = stmt->IDENTIFIER()->getText();

    // Check init expression
    auto exprs = stmt->expression();
    auto* initType = resolveExprType(exprs[0]);

    if (varType && initType && !isAssignable(varType, initType))
        error(stmt, "for init type mismatch: expected '" +
                         varType->name + "', got '" + initType->name + "'");

    // Register the loop variable
    if (varType)
        locals_[varName] = {varType, dims};

    // Check condition expression
    resolveExprType(exprs[1]);

    // Check update expression
    resolveExprType(exprs[2]);

    loopDepth_++;
    checkBlock(stmt->block(), retType);
    loopDepth_--;

    locals_.erase(varName);
}

// ═══════════════════════════════════════════════════════════════════════
//  Statement checks
// ═══════════════════════════════════════════════════════════════════════

void Checker::checkVarDeclStmt(LuxParser::VarDeclStmtContext* stmt) {
    auto name = stmt->IDENTIFIER()->getText();

    if (locals_.count(name)) {
        error(stmt, "variable '" + name + "' already declared in this scope");
        return;
    }

    unsigned arrayDims = 0;
    auto* typeInfo = resolveTypeSpec(stmt->typeSpec(), arrayDims);
    if (!typeInfo) return;

    // Declaration without initializer: int32 x;
    if (!stmt->expression()) {
        locals_[name] = {typeInfo, arrayDims};
        return;
    }

    // Validate initializer expression
    auto* initType = resolveExprType(stmt->expression());

    // Allow array literal → vec conversion
    if (typeInfo->kind == TypeKind::Extended &&
        dynamic_cast<LuxParser::ArrayLitExprContext*>(stmt->expression())) {
        // Array literal initializing an extended type — validate element types
        auto* arrExpr = dynamic_cast<LuxParser::ArrayLitExprContext*>(
            stmt->expression());
        auto elems = arrExpr->expression();
        for (auto* e : elems) {
            auto* et = resolveExprType(e);
            if (et && typeInfo->elementType &&
                !isAssignable(typeInfo->elementType, et)) {
                error(e, "element type mismatch: expected '" +
                         typeInfo->elementType->name + "', got '" +
                         et->name + "'");
            }
        }
    } else if (initType && typeInfo && !isAssignable(typeInfo, initType)) {
        error(stmt, "type mismatch: cannot assign '" + initType->name +
                         "' to variable '" + name + "' of type '" +
                         typeInfo->name + "'");
    }

    locals_[name] = {typeInfo, arrayDims};
}

void Checker::checkAssignStmt(LuxParser::AssignStmtContext* stmt) {
    auto name = stmt->IDENTIFIER()->getText();
    auto it = locals_.find(name);

    if (it == locals_.end()) {
        error(stmt, "undefined variable '" + name + "'");
        return;
    }

    // Validate index expressions and RHS
    auto indexExprs = stmt->expression();

    auto* varType = it->second.type;

    // For Map<K,V>, subscript assignment: m[key] = val
    if (indexExprs.size() == 2 && varType &&
        varType->kind == TypeKind::Extended && varType->keyType) {
        auto* keyType = resolveExprType(indexExprs[0]);
        auto* rhsType = resolveExprType(indexExprs[1]);

        if (keyType && !isAssignable(varType->keyType, keyType))
            error(stmt, "map key type mismatch: expected '" +
                         varType->keyType->name + "', got '" + keyType->name + "'");
        if (rhsType && varType->valueType && !isAssignable(varType->valueType, rhsType))
            error(stmt, "map value type mismatch: expected '" +
                         varType->valueType->name + "', got '" + rhsType->name + "'");
        return;
    }

    for (size_t i = 0; i + 1 < indexExprs.size(); i++) {
        auto* idxType = resolveExprType(indexExprs[i]);
        if (idxType && !isInteger(idxType))
            error(stmt, "index must be integer, got '" +
                             idxType->name + "'");
    }

    if (!indexExprs.empty()) {
        auto* rhsType = resolveExprType(indexExprs.back());
        // For extended types (Vec<T>), validate against element type
        auto* expectedType = varType;
        if (indexExprs.size() > 1 && expectedType &&
            expectedType->kind == TypeKind::Extended && expectedType->elementType) {
            expectedType = expectedType->elementType;
        }
        if (rhsType && expectedType && !isAssignable(expectedType, rhsType)) {
            error(stmt, "type mismatch: cannot assign '" + rhsType->name +
                             "' to variable '" + name + "' of type '" +
                             expectedType->name + "'");
        }
    }
}

void Checker::checkCompoundAssignStmt(LuxParser::CompoundAssignStmtContext* stmt) {
    auto name = stmt->IDENTIFIER()->getText();
    auto it = locals_.find(name);

    if (it == locals_.end()) {
        error(stmt, "undefined variable '" + name + "'");
        return;
    }

    auto* varType = it->second.type;
    auto* rhsType = resolveExprType(stmt->expression());
    auto opText = stmt->op->getText();

    bool needsNumeric = (opText == "+=" || opText == "-=" ||
                         opText == "*=" || opText == "/=");
    bool needsInteger = (opText == "%=" || opText == "&=" || opText == "|=" ||
                         opText == "^=" || opText == "<<=" || opText == ">>=");

    if (needsNumeric && varType && !isNumeric(varType))
        error(stmt, "operator '" + opText +
                         "' requires numeric variable, got '" + varType->name + "'");
    if (needsInteger && varType && !isInteger(varType))
        error(stmt, "operator '" + opText +
                         "' requires integer variable, got '" + varType->name + "'");
    if (needsNumeric && rhsType && !isNumeric(rhsType))
        error(stmt, "operator '" + opText +
                         "' requires numeric operand, got '" + rhsType->name + "'");
    if (needsInteger && rhsType && !isInteger(rhsType))
        error(stmt, "operator '" + opText +
                         "' requires integer operand, got '" + rhsType->name + "'");
}

void Checker::checkFieldAssignStmt(LuxParser::FieldAssignStmtContext* stmt) {
    auto identifiers = stmt->IDENTIFIER();
    auto varName = identifiers[0]->getText();

    auto it = locals_.find(varName);
    if (it == locals_.end()) {
        error(stmt, "undefined variable '" + varName + "'");
        return;
    }

    // Walk the field chain: p.x.y = val
    auto* currentType = it->second.type;
    for (size_t i = 1; i < identifiers.size(); i++) {
        auto fieldName = identifiers[i]->getText();

        if (!currentType || (currentType->kind != TypeKind::Struct && currentType->kind != TypeKind::Union)) {
            error(stmt, "'" +
                             (i == 1 ? varName : identifiers[i-1]->getText()) +
                             "' is not a struct or union type");
            return;
        }

        bool found = false;
        for (auto& field : currentType->fields) {
            if (field.name == fieldName) {
                currentType = field.typeInfo;
                found = true;
                break;
            }
        }
        if (!found) {
            error(stmt, "struct '" + currentType->name +
                             "' has no field '" + fieldName + "'");
            return;
        }
    }

    auto* rhsType = resolveExprType(stmt->expression());
    if (rhsType && currentType && !isAssignable(currentType, rhsType)) {
        error(stmt, "type mismatch in field assignment: expected '" +
                         currentType->name + "', got '" + rhsType->name + "'");
    }
}

void Checker::checkDerefAssignStmt(LuxParser::DerefAssignStmtContext* stmt) {
    if (stmt->IDENTIFIER()) {
        // *ptr = value;
        auto varName = stmt->IDENTIFIER()->getText();
        auto it = locals_.find(varName);

        if (it == locals_.end()) {
            error(stmt, "undefined variable '" + varName + "'");
            return;
        }

        if (it->second.type && it->second.type->kind != TypeKind::Pointer) {
            error(stmt, "cannot dereference non-pointer variable '" +
                             varName + "' of type '" + it->second.type->name + "'");
        }

        resolveExprType(stmt->expression(0));
    } else {
        // *(expr) = value;
        auto* ptrType = resolveExprType(stmt->expression(0));
        if (ptrType && ptrType->kind != TypeKind::Pointer) {
            error(stmt, "cannot dereference non-pointer expression of type '" +
                             ptrType->name + "'");
        }
        resolveExprType(stmt->expression(1));
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Global builtins — always available without `use`
// ═══════════════════════════════════════════════════════════════════════

void Checker::registerGlobalBuiltins() {
    auto* voidTy   = typeRegistry_.lookup("void");
    auto* i32Ty    = typeRegistry_.lookup("int32");
    auto* i64Ty    = typeRegistry_.lookup("int64");
    auto* f64Ty    = typeRegistry_.lookup("float64");
    auto* boolTy   = typeRegistry_.lookup("bool");
    auto* strTy    = typeRegistry_.lookup("string");

    // exit(int32) -> void
    functions_["exit"] = makeFunctionType(voidTy, { i32Ty });
    globalBuiltins_.insert("exit");

    // panic(string) -> void
    functions_["panic"] = makeFunctionType(voidTy, { strTy });
    globalBuiltins_.insert("panic");

    // assert(bool) -> void
    functions_["assert"] = makeFunctionType(voidTy, { boolTy });
    globalBuiltins_.insert("assert");

    // assertMsg(bool, string) -> void
    functions_["assertMsg"] = makeFunctionType(voidTy, { boolTy, strTy });
    globalBuiltins_.insert("assertMsg");

    // unreachable() -> void
    functions_["unreachable"] = makeFunctionType(voidTy, {});
    globalBuiltins_.insert("unreachable");

    // toInt(string) -> int64
    functions_["toInt"] = makeFunctionType(i64Ty, { strTy });
    globalBuiltins_.insert("toInt");

    // toFloat(string) -> float64
    functions_["toFloat"] = makeFunctionType(f64Ty, { strTy });
    globalBuiltins_.insert("toFloat");

    // toBool(string) -> bool
    functions_["toBool"] = makeFunctionType(boolTy, { strTy });
    globalBuiltins_.insert("toBool");

    // toString(T) -> string — accepts any primitive type
    // We register it with a dummy param; actual type checking is relaxed
    functions_["toString"] = makeFunctionType(strTy, { i32Ty });
    globalBuiltins_.insert("toString");

    // ── C FFI string conversion builtins ────────────────────────────
    auto* charPtrTy = getPointerType(typeRegistry_.lookup("char"));
    auto* usizeTy   = typeRegistry_.lookup("usize");

    // cstr(string) -> *char
    functions_["cstr"] = makeFunctionType(charPtrTy, { strTy });
    globalBuiltins_.insert("cstr");

    // fromCStr(*char) -> string
    functions_["fromCStr"] = makeFunctionType(strTy, { charPtrTy });
    globalBuiltins_.insert("fromCStr");

    // fromCStrLen(*char, usize) -> string
    functions_["fromCStrLen"] = makeFunctionType(strTy, { charPtrTy, usizeTy });
    globalBuiltins_.insert("fromCStrLen");
}

// ═══════════════════════════════════════════════════════════════════════
//  FFI: extern function declarations
// ═══════════════════════════════════════════════════════════════════════

void Checker::checkExternDecl(LuxParser::ExternDeclContext* decl) {
    auto funcName = decl->IDENTIFIER()->getText();

    unsigned retDims = 0;
    auto* retType = resolveTypeSpec(decl->typeSpec(), retDims);
    if (!retType) return;

    bool isVariadic = (decl->SPREAD() != nullptr);

    std::vector<const TypeInfo*> paramTypes;
    if (auto* paramList = decl->externParamList()) {
        for (auto* param : paramList->externParam()) {
            unsigned pDims = 0;
            auto* pType = resolveTypeSpec(param->typeSpec(), pDims);
            if (!pType) return;
            paramTypes.push_back(pType);
        }
    }

    auto* funcType = makeFunctionType(retType, paramTypes, isVariadic);
    functions_[funcName] = funcType;
    globalBuiltins_.insert(funcName);
}

void Checker::checkCallStmt(LuxParser::CallStmtContext* stmt) {
    auto name = stmt->IDENTIFIER()->getText();

    if (!isKnownFunction(name)) {
        error(stmt, "call to undeclared function '" + name +
                         "'. Did you forget 'use std::log::" + name + ";'?");
        return;
    }

    // Resolve all argument types
    std::vector<const TypeInfo*> argTypes;
    if (auto* argList = stmt->argList()) {
        for (auto* argExpr : argList->expression()) {
            argTypes.push_back(resolveExprType(argExpr));
        }
    }

    // Check user-defined function signatures
    auto fit = functions_.find(name);
    if (fit != functions_.end() && fit->second->kind == TypeKind::Function) {
        size_t argCount = argTypes.size();
        auto* fnType = fit->second;
        size_t paramCount = fnType->paramTypes.size();

        if (fnType->isVariadic) {
            // Variadic: need at least (paramCount - 1) args
            size_t minArgs = paramCount > 0 ? paramCount - 1 : 0;
            if (argCount < minArgs) {
                error(stmt, "function '" + name + "' expects at least " +
                                 std::to_string(minArgs) +
                                 " arguments, got " + std::to_string(argCount));
            }
            // Validate fixed parameter types
            for (size_t i = 0; i < std::min(argCount, paramCount); i++) {
                if (argTypes[i] && fnType->paramTypes[i] &&
                    !isAssignable(fnType->paramTypes[i], argTypes[i])) {
                    error(stmt,
                        "argument " + std::to_string(i + 1) +
                        " type mismatch in '" + name + "': expected '" +
                        fnType->paramTypes[i]->name + "', got '" +
                        argTypes[i]->name + "'");
                }
            }
        } else {
            if (argCount != paramCount) {
                error(stmt, "function '" + name + "' expects " +
                                 std::to_string(paramCount) +
                                 " arguments, got " + std::to_string(argCount));
            }
        }
        return;
    }

    // Check builtin function signatures
    auto* sig = builtinRegistry_.lookup(name);
    if (!sig) return; // unknown builtin, already reported

    size_t argCount = argTypes.size();
    if (sig->isVariadic) {
        if (argCount < sig->paramTypes.size()) {
            error(stmt, "'" + name + "' expects at least " +
                             std::to_string(sig->paramTypes.size()) +
                             " argument(s), got " + std::to_string(argCount));
            return;
        }
    } else if (argCount != sig->paramTypes.size()) {
        error(stmt, "'" + name + "' expects " +
                         std::to_string(sig->paramTypes.size()) +
                         " argument(s), got " + std::to_string(argCount));
        return;
    }

    // Validate argument types for non-polymorphic builtins
    if (!sig->isPolymorphic) {
        for (size_t i = 0; i < argCount; i++) {
            if (!argTypes[i]) continue;
            auto& expected = sig->paramTypes[i];
            auto* argTI = argTypes[i];

            if (expected == "_any" || expected == "_numeric" ||
                expected == "_integer" || expected == "_float")
                continue;

            auto* expectedTI = resolveBuiltinReturnType(expected);
            if (expectedTI && !isAssignable(expectedTI, argTI)) {
                error(stmt, "'" + name + "' argument " +
                    std::to_string(i + 1) + ": expected '" +
                    expected + "', got '" + argTI->name + "'");
            }
        }
    }
}

void Checker::checkExprStmt(LuxParser::ExprStmtContext* stmt) {
    resolveExprType(stmt->expression());
}

void Checker::checkReturnStmt(LuxParser::ReturnStmtContext* stmt,
                               const TypeInfo* expectedType) {
    auto* expr = stmt->expression();

    if (!expr) {
        if (expectedType->kind != TypeKind::Void) {
            error(stmt,
                "function with return type '" + expectedType->name +
                "' must return a value");
        }
        return;
    }

    auto* exprType = resolveExprType(expr);
    if (exprType && expectedType && !isAssignable(expectedType, exprType)) {
        error(stmt, "return type mismatch: expected '" +
                         expectedType->name + "', got '" + exprType->name + "'");
    }
}

unsigned Checker::resolveExprArrayDims(LuxParser::ExpressionContext* expr) {
    if (auto* id = dynamic_cast<LuxParser::IdentExprContext*>(expr)) {
        auto it = locals_.find(id->IDENTIFIER()->getText());
        if (it != locals_.end())
            return it->second.arrayDims;
    }
    if (auto* idx = dynamic_cast<LuxParser::IndexExprContext*>(expr)) {
        unsigned baseDims = resolveExprArrayDims(idx->expression(0));
        return baseDims > 0 ? baseDims - 1 : 0;
    }
    return 0;
}
