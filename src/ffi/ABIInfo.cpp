#include "ffi/ABIInfo.h"

static bool collectHomogeneousFPLeaves(llvm::Type* ty,
                                       llvm::Type*& leafTy,
                                       uint64_t& leafCount) {
    if (!ty) return false;

    if (ty->isFloatTy() || ty->isDoubleTy()) {
        if (!leafTy) leafTy = ty;
        if (leafTy != ty) return false;
        leafCount += 1;
        return true;
    }

    if (auto* arrTy = llvm::dyn_cast<llvm::ArrayType>(ty)) {
        if (arrTy->getNumElements() == 0) return false;
        for (uint64_t i = 0; i < arrTy->getNumElements(); ++i) {
            if (!collectHomogeneousFPLeaves(arrTy->getElementType(), leafTy, leafCount))
                return false;
        }
        return true;
    }

    if (auto* stTy = llvm::dyn_cast<llvm::StructType>(ty)) {
        if (stTy->getNumElements() == 0) return false;
        for (unsigned i = 0; i < stTy->getNumElements(); ++i) {
            if (!collectHomogeneousFPLeaves(stTy->getElementType(i), leafTy, leafCount))
                return false;
        }
        return true;
    }

    return false;
}

static bool isHomogeneousFPAggregate(llvm::Type* ty) {
    llvm::Type* leafTy = nullptr;
    uint64_t leafCount = 0;
    return collectHomogeneousFPLeaves(ty, leafTy, leafCount) && leafCount > 0;
}

static llvm::Type* getSingleEightbyteFPCoerceType(llvm::StructType* ty,
                                                   const llvm::DataLayout& dl,
                                                   llvm::LLVMContext& ctx) {
    if (!ty) return nullptr;

    uint64_t size = dl.getTypeAllocSize(ty);
    if (size == 0 || size > 8) return nullptr;

    llvm::Type* leafTy = nullptr;
    uint64_t leafCount = 0;
    if (!collectHomogeneousFPLeaves(ty, leafTy, leafCount) || !leafTy || leafCount == 0)
        return nullptr;

    uint64_t leafSize = dl.getTypeAllocSize(leafTy);
    if (leafSize == 0) return nullptr;

    // Avoid coercing aggregates with padding/holes.
    if (leafCount * leafSize != size) return nullptr;

    if (leafCount == 1) return leafTy;
    return llvm::FixedVectorType::get(leafTy, leafCount);
}

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

    // Homogeneous FP aggregates that fit in one eightbyte should use SSE-class
    // coercion to a scalar/vector FP type, not INTEGER-class iN coercion.
    if (auto* fpCoerceTy = getSingleEightbyteFPCoerceType(ty, dl, ctx)) {
        return { ABIArgKind::Coerce, fpCoerceTy, {} };
    }

    // For other homogeneous FP aggregates up to 16 bytes, prefer direct
    // lowering and let LLVM select the target ABI register class.
    if (size <= 16 && isHomogeneousFPAggregate(ty))
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
