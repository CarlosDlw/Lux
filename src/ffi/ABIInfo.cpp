#include "ffi/ABIInfo.h"

// ── x86-64 System V ABI struct classification ───────────────────────────────
//
// Reference: AMD64 ABI Draft 0.99.7, Section 3.2.3
//
// Structs are classified by size:
//   ≤  8 bytes → INTEGER class, one register → coerce to iN
//   9-16 bytes → INTEGER class, two registers → coerce/expand to {i64, iM}
//   > 16 bytes → MEMORY class → sret (return) or byval (param)

ABIArgInfo ABIClassifier::classifyReturn(llvm::Type* ty,
                                          const llvm::DataLayout& dl,
                                          llvm::LLVMContext& ctx) {
    if (!ty->isStructTy())
        return { ABIArgKind::Direct, nullptr, {} };

    return classifyStruct(llvm::cast<llvm::StructType>(ty), dl, ctx, true);
}

ABIArgInfo ABIClassifier::classifyParam(llvm::Type* ty,
                                         const llvm::DataLayout& dl,
                                         llvm::LLVMContext& ctx) {
    if (!ty->isStructTy())
        return { ABIArgKind::Direct, nullptr, {} };

    return classifyStruct(llvm::cast<llvm::StructType>(ty), dl, ctx, false);
}

ABIArgInfo ABIClassifier::classifyStruct(llvm::StructType* ty,
                                          const llvm::DataLayout& dl,
                                          llvm::LLVMContext& ctx,
                                          bool isReturn) {
    uint64_t size = dl.getTypeAllocSize(ty);

    if (size == 0)
        return { ABIArgKind::Direct, nullptr, {} };

    if (size <= 8) {
        // Single eightbyte: coerce to iN
        auto* coerced = llvm::IntegerType::get(ctx, size * 8);
        return { ABIArgKind::Coerce, coerced, {} };
    }

    if (size <= 16) {
        // Two eightbytes: coerce to {i64, iM}
        uint64_t hiSize = size - 8;
        auto* loTy = llvm::Type::getInt64Ty(ctx);
        auto* hiTy = llvm::IntegerType::get(ctx, hiSize * 8);

        if (isReturn) {
            // Return as aggregate {i64, iM}
            auto* coerced = llvm::StructType::get(ctx, { loTy, hiTy });
            return { ABIArgKind::Coerce, coerced, {} };
        } else {
            // Expand into two separate parameters
            return { ABIArgKind::Expand, nullptr, { loTy, hiTy } };
        }
    }

    // > 16 bytes: indirect (sret for return, byval for param)
    return { ABIArgKind::Indirect, ty, {} };
}
