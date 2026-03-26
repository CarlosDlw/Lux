#include "types/TypeInfo.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>

llvm::Type* TypeInfo::toLLVMType(llvm::LLVMContext& ctx,
                                 const llvm::DataLayout& dl) const {
    // Fixed-size inline array: [N x elemTy]
    if (arraySize > 0 && elementType) {
        auto* elemTy = elementType->toLLVMType(ctx, dl);
        return llvm::ArrayType::get(elemTy, arraySize);
    }

    switch (kind) {
    case TypeKind::Bool:
        return llvm::Type::getInt1Ty(ctx);

    case TypeKind::Char:
        return llvm::Type::getInt8Ty(ctx);

    case TypeKind::Integer:
        if (bitWidth == 0)
            return dl.getIntPtrType(ctx); // isize / usize
        return llvm::Type::getIntNTy(ctx, bitWidth);

    case TypeKind::Float:
        if (bitWidth == 128)
            return llvm::Type::getFP128Ty(ctx);
        if (bitWidth == 80)
            return llvm::Type::getX86_FP80Ty(ctx);
        if (bitWidth == 64)
            return llvm::Type::getDoubleTy(ctx);
        return llvm::Type::getFloatTy(ctx);

    case TypeKind::Void:
        return llvm::Type::getVoidTy(ctx);

    case TypeKind::String:
        // String is a struct { ptr (to uint8 data), usize (len) }
        return llvm::StructType::get(ctx, {
            llvm::PointerType::getUnqual(ctx),
            dl.getIntPtrType(ctx)
        });

    case TypeKind::Struct:
        return llvm::StructType::getTypeByName(ctx, name);

    case TypeKind::Union:
        return llvm::StructType::getTypeByName(ctx, name);

    case TypeKind::Enum:
        return llvm::Type::getInt32Ty(ctx);

    case TypeKind::Pointer:
        return llvm::PointerType::getUnqual(ctx);

    case TypeKind::Function:
        // Function values are represented as opaque pointers (function pointers)
        return llvm::PointerType::getUnqual(ctx);

    case TypeKind::Extended: {
        if (extendedKind == "Task" || extendedKind == "Mutex") {
            return llvm::PointerType::getUnqual(ctx);
        }
        if (extendedKind == "Map") {
            auto* existing = llvm::StructType::getTypeByName(ctx, "tollvm_map_header");
            if (existing) return existing;
            auto* ptrTy   = llvm::PointerType::getUnqual(ctx);
            auto* usizeTy = dl.getIntPtrType(ctx);
            return llvm::StructType::create(ctx, {
                ptrTy, ptrTy, ptrTy, ptrTy,
                usizeTy, usizeTy, usizeTy, usizeTy
            }, "tollvm_map_header");
        }
        if (extendedKind == "Set") {
            auto* existing = llvm::StructType::getTypeByName(ctx, "tollvm_set_header");
            if (existing) return existing;
            auto* ptrTy   = llvm::PointerType::getUnqual(ctx);
            auto* usizeTy = dl.getIntPtrType(ctx);
            return llvm::StructType::create(ctx, {
                ptrTy, ptrTy, ptrTy,
                usizeTy, usizeTy, usizeTy
            }, "tollvm_set_header");
        }
        // All Vec<T> variants share the same LLVM struct: { ptr, usize, usize }
        auto* existing = llvm::StructType::getTypeByName(ctx, "tollvm_vec_header");
        if (existing) return existing;
        return llvm::StructType::create(ctx, {
            llvm::PointerType::getUnqual(ctx),
            dl.getIntPtrType(ctx),
            dl.getIntPtrType(ctx)
        }, "tollvm_vec_header");
    }
    }

    return llvm::Type::getInt32Ty(ctx); // unreachable fallback
}
