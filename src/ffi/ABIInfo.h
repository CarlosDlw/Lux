#pragma once

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>

#include <vector>

// ── x86-64 System V ABI struct classification ───────────────────────────────
//
// Small structs (≤ 8 bytes)  → coerced to iN (single register)
// Medium structs (9-16 bytes) → coerced or expanded into {i64, iM}
// Large structs (> 16 bytes)  → indirect (sret for returns, byval for params)

enum class ABIArgKind {
    Direct,    // no coercion — scalar or pointer type
    Coerce,    // coerce struct to integer type(s)
    Expand,    // expand struct into multiple scalar arguments
    Indirect,  // pass via pointer (sret / byval)
};

struct ABIArgInfo {
    ABIArgKind kind = ABIArgKind::Direct;
    llvm::Type* coercedType = nullptr;
    std::vector<llvm::Type*> expandedTypes;
};

// Per-function ABI metadata for struct coercion at call sites.
struct CABIInfo {
    ABIArgInfo returnInfo;
    std::vector<ABIArgInfo> paramInfos;

    // Original struct types (before coercion). Null for non-struct params.
    llvm::Type* originalRetType = nullptr;
    std::vector<llvm::Type*> originalParamTypes;
};

class ABIClassifier {
public:
    // Classify a type for use as a function return value.
    static ABIArgInfo classifyReturn(llvm::Type* ty,
                                     const llvm::DataLayout& dl,
                                     llvm::LLVMContext& ctx);

    // Classify a type for use as a function parameter.
    static ABIArgInfo classifyParam(llvm::Type* ty,
                                    const llvm::DataLayout& dl,
                                    llvm::LLVMContext& ctx);

private:
    static ABIArgInfo classifyStruct(llvm::StructType* ty,
                                     const llvm::DataLayout& dl,
                                     llvm::LLVMContext& ctx,
                                     bool isReturn);
};
