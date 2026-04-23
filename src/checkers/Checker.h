#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "generated/LuxParser.h"
#include "imports/ImportResolver.h"
#include "types/TypeRegistry.h"
#include "types/MethodRegistry.h"
#include "types/ExtendedTypeRegistry.h"
#include "types/BuiltinRegistry.h"
#include "namespace/NamespaceRegistry.h"
#include "lsp/Diagnostic.h"

class CBindings;

class Checker {
public:
    bool check(LuxParser::ProgramContext* tree);

    const std::vector<std::string>& errors() const { return errors_; }
    const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }

    // Set namespace context for cross-file symbol resolution.
    void setNamespaceContext(const NamespaceRegistry* registry,
                             const std::string& currentNamespace,
                             const std::string& currentFile);

    // Set C bindings from parsed #include headers.
    void setCBindings(const CBindings* bindings);


private:
    std::vector<std::string> errors_;
    std::vector<Diagnostic>  diagnostics_;
    ImportResolver           imports_;
    TypeRegistry             typeRegistry_;
    MethodRegistry           methodRegistry_;
    ExtendedTypeRegistry     extTypeRegistry_;
    BuiltinRegistry          builtinRegistry_;

    // Variable info tracked per-function scope
    struct VarInfo {
        const TypeInfo* type;
        unsigned arrayDims = 0;
        bool initialized = true;  // false when declared without initializer
        bool used = false;         // set to true when the variable is read
        antlr4::Token* declToken = nullptr; // for warning location
    };
    std::unordered_map<std::string, VarInfo> locals_;

    // Module-level function signatures (name → function TypeInfo)
    std::unordered_map<std::string, const TypeInfo*> functions_;

    // Global builtin names (always available, no import needed)
    std::unordered_set<std::string> globalBuiltins_;

    // Track loop nesting for break/continue validation
    unsigned loopDepth_ = 0;

    // Struct methods registered via `extend` blocks
    struct StructMethodInfo {
        std::string name;
        const TypeInfo* returnType;
        std::vector<const TypeInfo*> paramTypes; // excludes &self
        bool isStatic = false;
    };
    std::unordered_map<std::string, std::vector<StructMethodInfo>> structMethods_;

    // Dynamically created TypeInfos (for pointer types, function types, etc.)
    std::vector<std::unique_ptr<TypeInfo>> dynamicTypes_;

    // ── Type resolution ──────────────────────────────────────────────
    const TypeInfo* resolveTypeSpec(LuxParser::TypeSpecContext* ctx,
                                   unsigned& arrayDims);
    const TypeInfo* resolveExprType(LuxParser::ExpressionContext* expr);
    const TypeInfo* getPointerType(const TypeInfo* pointee);
    const TypeInfo* makeFunctionType(const TypeInfo* returnType,
                                     const std::vector<const TypeInfo*>& paramTypes,
                                     bool isVariadic = false);
    std::string resolveBaseTypeName(LuxParser::TypeSpecContext* ctx);
    const TypeInfo* resolveBuiltinReturnType(const std::string& retName);

    // ── Type queries ─────────────────────────────────────────────────
    bool isNumeric(const TypeInfo* ti);
    bool isInteger(const TypeInfo* ti);
    bool isConditionType(const TypeInfo* ti);
    bool isAssignable(const TypeInfo* lhs, const TypeInfo* rhs);
    void checkNegativeToUnsigned(const TypeInfo* target,
                                 LuxParser::ExpressionContext* expr,
                                 antlr4::ParserRuleContext* ctx);
    unsigned resolveExprArrayDims(LuxParser::ExpressionContext* expr);

    // ── Top-level checks ─────────────────────────────────────────────
    void checkUseDecls(LuxParser::ProgramContext* tree);
    void checkTypeAliasDecl(LuxParser::TypeAliasDeclContext* decl);
    void checkStructDecl(LuxParser::StructDeclContext* decl);
    void checkUnionDecl(LuxParser::UnionDeclContext* decl);
    void checkEnumDecl(LuxParser::EnumDeclContext* decl);
    void checkExtendDecl(LuxParser::ExtendDeclContext* decl);
    void checkExtendMethodBodies(LuxParser::ExtendDeclContext* decl);
    void checkExternDecl(LuxParser::ExternDeclContext* decl);
    void registerFunctionSignature(LuxParser::FunctionDeclContext* func);
    void checkFunction(LuxParser::FunctionDeclContext* func);
    bool blockAlwaysReturns(LuxParser::BlockContext* block);
    void registerGlobalBuiltins();

    // ── Statement checks ────────────────────────────────────────────
    void checkBlock(LuxParser::BlockContext* block, const TypeInfo* retType);
    void checkStmt(LuxParser::StatementContext* stmt, const TypeInfo* retType, bool& terminated);
    void checkVarDeclStmt(LuxParser::VarDeclStmtContext* stmt);
    void checkAssignStmt(LuxParser::AssignStmtContext* stmt);
    void checkCompoundAssignStmt(LuxParser::CompoundAssignStmtContext* stmt);
    void checkFieldAssignStmt(LuxParser::FieldAssignStmtContext* stmt);
    void checkDerefAssignStmt(LuxParser::DerefAssignStmtContext* stmt);
    void checkCallStmt(LuxParser::CallStmtContext* stmt);
    void checkExprStmt(LuxParser::ExprStmtContext* stmt);
    void checkReturnStmt(LuxParser::ReturnStmtContext* stmt,
                         const TypeInfo* expectedType);
    void checkIfStmt(LuxParser::IfStmtContext* stmt,
                     const TypeInfo* retType);
    void checkForInStmt(LuxParser::ForInStmtContext* stmt,
                        const TypeInfo* retType);
    void checkForClassicStmt(LuxParser::ForClassicStmtContext* stmt,
                             const TypeInfo* retType);
    void checkSwitchStmt(LuxParser::SwitchStmtContext* stmt,
                         const TypeInfo* retType);

    // ── Flow analysis helpers ───────────────────────────────────────
    bool isTerminatorStmt(LuxParser::StatementContext* stmt);
    void warnUnusedLocals(LuxParser::FunctionDeclContext* func);
    void warnUnusedLocals(antlr4::ParserRuleContext* ctx);

    // ── Error reporting with source location ────────────────────────
    void error(antlr4::ParserRuleContext* ctx, const std::string& msg);
    void warning(antlr4::ParserRuleContext* ctx, const std::string& msg);
    void warningToken(antlr4::Token* start, antlr4::Token* stop,
                      const std::string& msg);
    void errorToken(antlr4::Token* start, antlr4::Token* stop,
                    const std::string& msg);
    void emitDiag(antlr4::Token* start, antlr4::Token* stop,
                  Diagnostic::Severity sev, const std::string& msg);

    // ── Namespace context (set by CLI before check) ─────────────────
    const NamespaceRegistry* nsRegistry_  = nullptr;
    std::string currentNamespace_;
    std::string currentFile_;

    // User imports: symbol name → namespace it was imported from
    std::unordered_map<std::string, std::string> userImports_;

    // C bindings from parsed #include headers
    const CBindings* cBindings_ = nullptr;

    // C enum constants: name → { type, value }
    struct CEnumConstant {
        const TypeInfo* type;
        int64_t value = 0;
        double  floatValue = 0.0;
        bool    isFloat  = false;
        bool    isString = false;
    };
    std::unordered_map<std::string, CEnumConstant> cEnumConstants_;

    // C global variables: name → type
    std::unordered_map<std::string, const TypeInfo*> cGlobals_;

    // Check if a function is known (local, same-namespace, imported, or builtin)
    bool isKnownFunction(const std::string& name) const;
    bool isKnownType(const std::string& name) const;

    // ── User-defined generics (monomorphization) ─────────────────────
    struct GenericStructTemplate {
        std::vector<std::string>      typeParams;  // e.g. {"T"} or {"K", "V"}
        LuxParser::StructDeclContext* decl;        // original AST node (non-owning)
    };
    struct GenericFuncTemplate {
        std::vector<std::string>       typeParams;
        LuxParser::FunctionDeclContext* decl;
    };
    struct GenericExtendTemplate {
        std::vector<std::string>       typeParams;
        LuxParser::ExtendDeclContext*  decl;
    };
    std::unordered_map<std::string, GenericStructTemplate>  genericStructTemplates_;
    std::unordered_map<std::string, GenericFuncTemplate>    genericFuncTemplates_;
    std::unordered_map<std::string, GenericExtendTemplate>  genericExtendTemplates_;
    // Tracks instantiations in progress for cycle detection
    std::unordered_set<std::string> instantiatingGenerics_;

    // Mangles "Node" + ["int32"] → "Node__int32"
    static std::string mangleGenericName(const std::string& baseName,
                                         const std::vector<const TypeInfo*>& typeArgs);

    // Produces (and caches) a concrete TypeInfo for a generic struct instantiation.
    // Returns nullptr and emits an error on failure.
    const TypeInfo* instantiateGenericStruct(
        const std::string& baseName,
        const GenericStructTemplate& tmpl,
        const std::vector<const TypeInfo*>& typeArgs,
        antlr4::ParserRuleContext* ctx);

    // Produces (and caches) a concrete TypeInfo for a generic function instantiation.
    const TypeInfo* instantiateGenericFunc(
        const std::string& baseName,
        const GenericFuncTemplate& tmpl,
        const std::vector<const TypeInfo*>& typeArgs,
        antlr4::ParserRuleContext* ctx);

    // Resolves a type spec under a substitution map (typeParam → concrete TypeInfo*).
    const TypeInfo* resolveTypeSpecWithSubst(
        LuxParser::TypeSpecContext* typeSpec,
        const std::unordered_map<std::string, const TypeInfo*>& subst,
        unsigned& arrayDims);

    // Returns the constraint TypeInfo for a type param (e.g. "numeric"), or nullptr.
    const TypeInfo* resolveTypeParamConstraint(const std::string& constraintName,
                                               antlr4::ParserRuleContext* ctx);

    // Checks that a concrete type arg satisfies its constraint.
    bool satisfiesConstraint(const TypeInfo* typeArg,
                             const TypeInfo* constraint,
                             const std::string& paramName,
                             antlr4::ParserRuleContext* ctx);
};
