#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "generated/ToLLVMParser.h"
#include "imports/ImportResolver.h"
#include "types/TypeRegistry.h"
#include "types/MethodRegistry.h"
#include "types/ExtendedTypeRegistry.h"
#include "types/BuiltinRegistry.h"
#include "namespace/NamespaceRegistry.h"

class CBindings;

class Checker {
public:
    bool check(ToLLVMParser::ProgramContext* tree);

    const std::vector<std::string>& errors() const { return errors_; }

    // Set namespace context for cross-file symbol resolution.
    void setNamespaceContext(const NamespaceRegistry* registry,
                             const std::string& currentNamespace,
                             const std::string& currentFile);

    // Set C bindings from parsed #include headers.
    void setCBindings(const CBindings* bindings);


private:
    std::vector<std::string> errors_;
    ImportResolver           imports_;
    TypeRegistry             typeRegistry_;
    MethodRegistry           methodRegistry_;
    ExtendedTypeRegistry     extTypeRegistry_;
    BuiltinRegistry          builtinRegistry_;

    // Variable info tracked per-function scope
    struct VarInfo {
        const TypeInfo* type;
        unsigned arrayDims = 0;
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
    const TypeInfo* resolveTypeSpec(ToLLVMParser::TypeSpecContext* ctx,
                                   unsigned& arrayDims);
    const TypeInfo* resolveExprType(ToLLVMParser::ExpressionContext* expr);
    const TypeInfo* getPointerType(const TypeInfo* pointee);
    const TypeInfo* makeFunctionType(const TypeInfo* returnType,
                                     const std::vector<const TypeInfo*>& paramTypes,
                                     bool isVariadic = false);
    std::string resolveBaseTypeName(ToLLVMParser::TypeSpecContext* ctx);
    const TypeInfo* resolveBuiltinReturnType(const std::string& retName);

    // ── Type queries ─────────────────────────────────────────────────
    bool isNumeric(const TypeInfo* ti);
    bool isInteger(const TypeInfo* ti);
    bool isAssignable(const TypeInfo* lhs, const TypeInfo* rhs);
    unsigned resolveExprArrayDims(ToLLVMParser::ExpressionContext* expr);

    // ── Top-level checks ─────────────────────────────────────────────
    void checkUseDecls(ToLLVMParser::ProgramContext* tree);
    void checkTypeAliasDecl(ToLLVMParser::TypeAliasDeclContext* decl);
    void checkStructDecl(ToLLVMParser::StructDeclContext* decl);
    void checkUnionDecl(ToLLVMParser::UnionDeclContext* decl);
    void checkEnumDecl(ToLLVMParser::EnumDeclContext* decl);
    void checkExtendDecl(ToLLVMParser::ExtendDeclContext* decl);
    void checkExternDecl(ToLLVMParser::ExternDeclContext* decl);
    void registerFunctionSignature(ToLLVMParser::FunctionDeclContext* func);
    void checkFunction(ToLLVMParser::FunctionDeclContext* func);
    void registerGlobalBuiltins();

    // ── Statement checks ────────────────────────────────────────────
    void checkBlock(ToLLVMParser::BlockContext* block, const TypeInfo* retType);
    void checkVarDeclStmt(ToLLVMParser::VarDeclStmtContext* stmt);
    void checkAssignStmt(ToLLVMParser::AssignStmtContext* stmt);
    void checkCompoundAssignStmt(ToLLVMParser::CompoundAssignStmtContext* stmt);
    void checkFieldAssignStmt(ToLLVMParser::FieldAssignStmtContext* stmt);
    void checkDerefAssignStmt(ToLLVMParser::DerefAssignStmtContext* stmt);
    void checkCallStmt(ToLLVMParser::CallStmtContext* stmt);
    void checkExprStmt(ToLLVMParser::ExprStmtContext* stmt);
    void checkReturnStmt(ToLLVMParser::ReturnStmtContext* stmt,
                         const TypeInfo* expectedType);
    void checkIfStmt(ToLLVMParser::IfStmtContext* stmt,
                     const TypeInfo* retType);
    void checkForInStmt(ToLLVMParser::ForInStmtContext* stmt,
                        const TypeInfo* retType);
    void checkForClassicStmt(ToLLVMParser::ForClassicStmtContext* stmt,
                             const TypeInfo* retType);
    void checkSwitchStmt(ToLLVMParser::SwitchStmtContext* stmt,
                         const TypeInfo* retType);

    // ── Error reporting with source location ────────────────────────
    void error(antlr4::ParserRuleContext* ctx, const std::string& msg);

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
        int64_t value;
    };
    std::unordered_map<std::string, CEnumConstant> cEnumConstants_;

    // C global variables: name → type
    std::unordered_map<std::string, const TypeInfo*> cGlobals_;

    // Check if a function is known (local, same-namespace, imported, or builtin)
    bool isKnownFunction(const std::string& name) const;
    bool isKnownType(const std::string& name) const;
};
