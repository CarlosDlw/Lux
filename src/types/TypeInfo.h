#pragma once

#include <string>
#include <vector>

namespace llvm {
class Type;
class LLVMContext;
class DataLayout;
}

enum class TypeKind {
    Integer,
    Float,
    Bool,
    Char,
    Void,
    String,
    Struct, // user-defined struct types
    Union,  // user-defined union types (fields share memory)
    Enum,   // user-defined enum types
    Pointer, // pointer to another type
    Function, // function type: fn(params) -> ret
    Extended, // extended types from stdlib (Vec<T>, Map<K,V>, etc.)
};

struct TypeInfo;

struct FieldInfo {
    std::string     name;
    const TypeInfo* typeInfo;
};

struct TypeInfo {
    std::string name;           // language name: "int32", "uint64", "float", etc.
    TypeKind    kind;
    unsigned    bitWidth;       // for Integer: 1,8,16,32,64,128,256. 0 = pointer-sized (isize/usize)
    bool        isSigned;       // meaningful for Integer only
    std::string builtinSuffix;  // suffix for C builtins: "i32", "u64", "f32", "str", "bool"
    std::vector<FieldInfo> fields;          // non-empty only for Struct
    std::vector<std::string> enumVariants;  // non-empty only for Enum
    const TypeInfo* pointeeType = nullptr;   // non-null only for Pointer
    const TypeInfo* returnType = nullptr;                // non-null only for Function
    std::vector<const TypeInfo*> paramTypes;              // non-empty only for Function
    bool isVariadic = false;                             // true if last param is variadic (...)
    const TypeInfo* elementType = nullptr;   // non-null for Extended generics (Vec<T> → T)
    const TypeInfo* keyType = nullptr;       // non-null for Map<K,V> → K
    const TypeInfo* valueType = nullptr;     // non-null for Map<K,V> → V
    std::string extendedKind;                // extended type base name: "Vec", "Map", etc.
    unsigned arraySize = 0;                  // >0 for fixed-size arrays: [N]T

    llvm::Type* toLLVMType(llvm::LLVMContext& ctx,
                           const llvm::DataLayout& dl) const;
};
