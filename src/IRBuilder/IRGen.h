#pragma once

#include <memory>
#include <string>
#include <any>
#include <unordered_map>
#include <unordered_set>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>

#include "generated/ToLLVMParserBaseVisitor.h"
#include "generated/ToLLVMParser.h"
#include "LLVM_IR/IRModule.h"
#include "imports/ImportResolver.h"
#include "types/TypeRegistry.h"
#include "types/MethodRegistry.h"
#include "types/ExtendedTypeRegistry.h"
#include "ffi/ABIInfo.h"

class NamespaceRegistry;
class CBindings;
struct CStructMacro;

class IRGen : public ToLLVMParserBaseVisitor {
public:
    IRGen() = default;

    // Walk the parse tree and produce an IRModule.
    std::unique_ptr<IRModule> generate(ToLLVMParser::ProgramContext* tree,
                                       const std::string& moduleName);

    // Set namespace context for cross-file symbol resolution and mangling.
    void setNamespaceContext(const NamespaceRegistry* registry,
                             const std::string& currentNamespace,
                             const std::string& currentFile);

    // Set C bindings from parsed #include headers.
    void setCBindings(const CBindings* bindings);

    // ── Visitor overrides ───────────────────────────────────────────────────
    std::any visitProgram(ToLLVMParser::ProgramContext* ctx)             override;
    std::any visitStructDecl(ToLLVMParser::StructDeclContext* ctx)       override;
    std::any visitUnionDecl(ToLLVMParser::UnionDeclContext* ctx)         override;
    std::any visitEnumDecl(ToLLVMParser::EnumDeclContext* ctx)           override;
    std::any visitExternDecl(ToLLVMParser::ExternDeclContext* ctx)       override;
    std::any visitFunctionDecl(ToLLVMParser::FunctionDeclContext* ctx)   override;
    std::any visitBlock(ToLLVMParser::BlockContext* ctx)                 override;
    std::any visitUseItem(ToLLVMParser::UseItemContext* ctx)             override;
    std::any visitUseGroup(ToLLVMParser::UseGroupContext* ctx)           override;
    std::any visitVarDeclStmt(ToLLVMParser::VarDeclStmtContext* ctx)     override;
    std::any visitAssignStmt(ToLLVMParser::AssignStmtContext* ctx)       override;
    std::any visitCompoundAssignStmt(ToLLVMParser::CompoundAssignStmtContext* ctx) override;
    std::any visitFieldAssignStmt(ToLLVMParser::FieldAssignStmtContext* ctx) override;
    std::any visitIndexFieldAssignStmt(ToLLVMParser::IndexFieldAssignStmtContext* ctx) override;
    std::any visitArrowAssignStmt(ToLLVMParser::ArrowAssignStmtContext* ctx) override;
    std::any visitArrowCompoundAssignStmt(ToLLVMParser::ArrowCompoundAssignStmtContext* ctx) override;
    std::any visitCallStmt(ToLLVMParser::CallStmtContext* ctx)           override;
    std::any visitExprStmt(ToLLVMParser::ExprStmtContext* ctx)           override;
    std::any visitReturnStmt(ToLLVMParser::ReturnStmtContext* ctx)       override;
    std::any visitIfStmt(ToLLVMParser::IfStmtContext* ctx)               override;
    std::any visitForInStmt(ToLLVMParser::ForInStmtContext* ctx)         override;
    std::any visitForClassicStmt(ToLLVMParser::ForClassicStmtContext* ctx) override;
    std::any visitBreakStmt(ToLLVMParser::BreakStmtContext* ctx)         override;
    std::any visitContinueStmt(ToLLVMParser::ContinueStmtContext* ctx)   override;
    std::any visitLoopStmt(ToLLVMParser::LoopStmtContext* ctx)           override;
    std::any visitWhileStmt(ToLLVMParser::WhileStmtContext* ctx)         override;
    std::any visitDoWhileStmt(ToLLVMParser::DoWhileStmtContext* ctx)     override;
    std::any visitSwitchStmt(ToLLVMParser::SwitchStmtContext* ctx)       override;
    std::any visitMethodCallExpr(ToLLVMParser::MethodCallExprContext* ctx) override;
    std::any visitTypeSpec(ToLLVMParser::TypeSpecContext* ctx)           override;

    // Expression visitors (labeled alternatives)
    std::any visitIntLitExpr(ToLLVMParser::IntLitExprContext* ctx)       override;
    std::any visitFloatLitExpr(ToLLVMParser::FloatLitExprContext* ctx)   override;
    std::any visitBoolLitExpr(ToLLVMParser::BoolLitExprContext* ctx)     override;
    std::any visitCharLitExpr(ToLLVMParser::CharLitExprContext* ctx)     override;
    std::any visitStrLitExpr(ToLLVMParser::StrLitExprContext* ctx)       override;
    std::any visitCStrLitExpr(ToLLVMParser::CStrLitExprContext* ctx)   override;
    std::any visitIdentExpr(ToLLVMParser::IdentExprContext* ctx)         override;
    std::any visitArrayLitExpr(ToLLVMParser::ArrayLitExprContext* ctx)   override;
    std::any visitListCompExpr(ToLLVMParser::ListCompExprContext* ctx)   override;
    std::any visitIndexExpr(ToLLVMParser::IndexExprContext* ctx)         override;
    std::any visitStructLitExpr(ToLLVMParser::StructLitExprContext* ctx) override;
    std::any visitFieldAccessExpr(ToLLVMParser::FieldAccessExprContext* ctx) override;
    std::any visitArrowAccessExpr(ToLLVMParser::ArrowAccessExprContext* ctx) override;
    std::any visitEnumAccessExpr(ToLLVMParser::EnumAccessExprContext* ctx) override;
    std::any visitStaticMethodCallExpr(ToLLVMParser::StaticMethodCallExprContext* ctx) override;
    std::any visitNullLitExpr(ToLLVMParser::NullLitExprContext* ctx)     override;
    std::any visitAddrOfExpr(ToLLVMParser::AddrOfExprContext* ctx)       override;
    std::any visitDerefExpr(ToLLVMParser::DerefExprContext* ctx)         override;
    std::any visitDerefAssignStmt(ToLLVMParser::DerefAssignStmtContext* ctx) override;
    std::any visitTypeAliasDecl(ToLLVMParser::TypeAliasDeclContext* ctx) override;
    std::any visitFnCallExpr(ToLLVMParser::FnCallExprContext* ctx)       override;
    std::any visitNegExpr(ToLLVMParser::NegExprContext* ctx)             override;
    std::any visitMulExpr(ToLLVMParser::MulExprContext* ctx)             override;
    std::any visitAddSubExpr(ToLLVMParser::AddSubExprContext* ctx)       override;
    std::any visitRelExpr(ToLLVMParser::RelExprContext* ctx)             override;
    std::any visitEqExpr(ToLLVMParser::EqExprContext* ctx)               override;
    std::any visitLogicalNotExpr(ToLLVMParser::LogicalNotExprContext* ctx) override;
    std::any visitLogicalAndExpr(ToLLVMParser::LogicalAndExprContext* ctx) override;
    std::any visitLogicalOrExpr(ToLLVMParser::LogicalOrExprContext* ctx) override;
    std::any visitBitNotExpr(ToLLVMParser::BitNotExprContext* ctx)       override;
    std::any visitShiftExpr(ToLLVMParser::ShiftExprContext* ctx)         override;
    std::any visitBitAndExpr(ToLLVMParser::BitAndExprContext* ctx)       override;
    std::any visitBitXorExpr(ToLLVMParser::BitXorExprContext* ctx)       override;
    std::any visitBitOrExpr(ToLLVMParser::BitOrExprContext* ctx)         override;
    std::any visitPreIncrExpr(ToLLVMParser::PreIncrExprContext* ctx)     override;
    std::any visitPreDecrExpr(ToLLVMParser::PreDecrExprContext* ctx)     override;
    std::any visitPostIncrExpr(ToLLVMParser::PostIncrExprContext* ctx)   override;
    std::any visitPostDecrExpr(ToLLVMParser::PostDecrExprContext* ctx)   override;
    std::any visitCastExpr(ToLLVMParser::CastExprContext* ctx)           override;
    std::any visitSizeofExpr(ToLLVMParser::SizeofExprContext* ctx)       override;
    std::any visitTypeofExpr(ToLLVMParser::TypeofExprContext* ctx)      override;
    std::any visitTernaryExpr(ToLLVMParser::TernaryExprContext* ctx)     override;
    std::any visitIsExpr(ToLLVMParser::IsExprContext* ctx)               override;
    std::any visitNullCoalExpr(ToLLVMParser::NullCoalExprContext* ctx)   override;
    std::any visitRangeExpr(ToLLVMParser::RangeExprContext* ctx)         override;
    std::any visitRangeInclExpr(ToLLVMParser::RangeInclExprContext* ctx) override;
    std::any visitSpreadExpr(ToLLVMParser::SpreadExprContext* ctx)       override;
    std::any visitParenExpr(ToLLVMParser::ParenExprContext* ctx)         override;
    std::any visitSpawnExpr(ToLLVMParser::SpawnExprContext* ctx)         override;
    std::any visitAwaitExpr(ToLLVMParser::AwaitExprContext* ctx)         override;
    std::any visitLockStmt(ToLLVMParser::LockStmtContext* ctx)           override;
    std::any visitTryCatchStmt(ToLLVMParser::TryCatchStmtContext* ctx)   override;
    std::any visitThrowStmt(ToLLVMParser::ThrowStmtContext* ctx)         override;
    std::any visitTryExpr(ToLLVMParser::TryExprContext* ctx)             override;
    std::any visitExtendDecl(ToLLVMParser::ExtendDeclContext* ctx)        override;
    std::any visitDeferStmt(ToLLVMParser::DeferStmtContext* ctx)           override;

private:
    // Non-owning pointers valid only during generate().
    llvm::LLVMContext*  context_         = nullptr;
    llvm::Module*       module_          = nullptr;
    llvm::IRBuilder<>*  builder_         = nullptr;
    llvm::Function*     currentFunction_ = nullptr;

    // Symbol table: variable name → {alloca, type info}
    struct VarInfo {
        llvm::AllocaInst* alloca;
        const TypeInfo*   typeInfo;
        unsigned          arrayDims = 0; // 0 = scalar, 1 = []T, 2 = [][]T, etc.
        bool              isParam   = false; // true = borrowed from caller, skip auto-free
    };
    std::unordered_map<std::string, VarInfo> locals_;

    // Variadic parameter info (name → {data ptr alloca, len alloca, element type})
    struct VariadicParamInfo {
        llvm::AllocaInst* dataPtr;      // alloca storing pointer to the data array
        llvm::AllocaInst* lenAlloca;    // alloca storing the element count (i64)
        const TypeInfo*   elementType;  // type of each element
    };
    std::unordered_map<std::string, VariadicParamInfo> variadicParams_;

    // Tracks which functions are variadic and the index of their variadic param
    struct VariadicFuncInfo {
        size_t variadicParamIdx;    // index in the grammar param list
        const TypeInfo* elementType; // type of each variadic element
    };
    std::unordered_map<std::string, VariadicFuncInfo> variadicFunctions_;

    // Import resolver for `use` declarations
    ImportResolver imports_;

    // Global builtins (always available, no `use` required)
    std::unordered_set<std::string> globalBuiltins_;

    // Extern C functions declared via FFI `extern` keyword
    std::unordered_set<std::string> externCFunctions_;

    // Centralized type registry
    TypeRegistry typeRegistry_;

    // Built-in type method registry
    MethodRegistry methodRegistry_;

    // Extended type registry (vec, map, etc.)
    ExtendedTypeRegistry extTypeRegistry_;

    // Loop break/continue target stack
    struct LoopInfo {
        llvm::BasicBlock* breakTarget;
        llvm::BasicBlock* continueTarget;
    };
    std::vector<LoopInfo> loopStack_;

    // Deferred statements (function-scoped, LIFO execution order)
    struct DeferredStmt {
        ToLLVMParser::CallStmtContext* callCtx = nullptr;
        ToLLVMParser::ExprStmtContext* exprCtx = nullptr;
    };
    std::vector<DeferredStmt> deferStack_;

    // Spawn counter for unique wrapper function names
    unsigned spawnCounter_ = 0;

    // Struct methods registered via `extend` blocks
    // structName → { methodName → Function* }
    std::unordered_map<std::string,
        std::unordered_map<std::string, llvm::Function*>> structMethods_;

    // Static struct methods (no &self)
    std::unordered_map<std::string,
        std::unordered_map<std::string, llvm::Function*>> staticStructMethods_;

    // Namespace context for cross-file resolution
    const NamespaceRegistry* nsRegistry_ = nullptr;
    std::string currentNamespace_;
    std::string currentFile_;

    // Maps unmangled symbol name → mangled LLVM function name
    // Built during visitProgram to resolve calls efficiently.
    std::unordered_map<std::string, std::string> callTargetMap_;

    // User imports: symbol name → source namespace
    std::unordered_map<std::string, std::string> userImports_;

    // C bindings from parsed #include headers
    const CBindings* cBindings_ = nullptr;

    // C enum constants: qualified name → integer value
    std::unordered_map<std::string, int64_t> cEnumConstants_;

    // C struct literal macros: name → CStructMacro pointer (owned by CBindings)
    std::unordered_map<std::string, const CStructMacro*> cStructMacros_;

    // C global variables: name → { LLVM global, TypeInfo }
    struct CGlobalEntry {
        llvm::GlobalVariable* global;
        const TypeInfo* type;
    };
    std::unordered_map<std::string, CGlobalEntry> cGlobals_;

    // ABI coercion info for C functions that pass/return structs by value
    std::unordered_map<std::string, CABIInfo> cabiInfos_;

    // Lazily declare a C function from CBindings when first called.
    llvm::Function* declareCFunction(const std::string& name);

    // Pre-processing helpers for cross-file symbols
    void registerCrossFileSymbols(ToLLVMParser::ProgramContext* ctx);
    void declareExternFunction(const std::string& mangledName,
                               ToLLVMParser::FunctionDeclContext* decl);

    // Forward-declare a user function (signature only, no body)
    void forwardDeclareFunction(ToLLVMParser::FunctionDeclContext* decl);
    std::string resolveCallTarget(const std::string& name) const;

    // Helpers
    const TypeInfo*    resolveTypeInfo(ToLLVMParser::TypeSpecContext* ctx);
    const TypeInfo*    resolveExprTypeInfo(ToLLVMParser::ExpressionContext* ctx);
    bool               isPointerType(ToLLVMParser::TypeSpecContext* ctx);
    unsigned           countArrayDims(ToLLVMParser::TypeSpecContext* ctx);
    llvm::ArrayType*   buildTargetArrayType(llvm::Type* srcTy, llvm::Type* elemTy);
    void               storeArrayElements(llvm::Value* src, llvm::Value* destPtr,
                                          llvm::Type* destArrTy, const TypeInfo* elemTI,
                                          unsigned dims);
    llvm::FunctionCallee declareBuiltin(const std::string& name,
                                        llvm::Type* retType,
                                        llvm::ArrayRef<llvm::Type*> argTypes);
    llvm::StructType*   getOrCreateVecStructType();
    llvm::StructType*   getOrCreateMapStructType();
    llvm::StructType*   getOrCreateSetStructType();
    std::string         getVecSuffix(const TypeInfo* elemTI);
    void                emitDeferredCleanups();
    void                emitAutoCleanups(const std::string& skipVar = "");
    void                emitAllCleanups(const std::string& skipVar = "");
    std::pair<llvm::Value*, llvm::Value*>
                       promoteArithmetic(llvm::Value* lhs, llvm::Value* rhs);
};
