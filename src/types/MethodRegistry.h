#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "types/TypeInfo.h"

// Describes a single built-in method on a primitive/built-in type.
struct MethodDescriptor {
    std::string              name;        // method name: "abs", "len", "contains", etc.
    std::vector<TypeKind>    receiverKinds; // which TypeKinds this method applies to
    std::vector<std::string> paramTypes;  // parameter type names (empty = no args)
    std::string              returnType;  // return type name ("_self" = same as receiver)

    // Constraints for more specific matching (optional)
    bool requireSigned   = false; // only signed integers (e.g. abs, sign)
    bool requireUnsigned = false; // only unsigned integers
    bool requireNumeric  = false; // only numeric array elements (e.g. sum, average)
    bool isArrayMethod   = false; // method for []T arrays (uses _elem convention)

    // A unique identifier used by IRGen to dispatch codegen
    std::string emitTag;          // e.g. "int.abs", "float.sqrt", "string.len"
};

// Central registry of all built-in type methods.
// Both the Checker and IRGen query this to validate and emit method calls.
class MethodRegistry {
public:
    MethodRegistry();

    // Look up a method by receiver TypeKind + method name.
    // Returns nullptr if not found.
    const MethodDescriptor* lookup(TypeKind receiverKind,
                                   const std::string& methodName) const;

    // Look up a method for array types (identified by arrayDims > 0).
    const MethodDescriptor* lookupArrayMethod(const std::string& methodName) const;

    // Get all methods for a given TypeKind (useful for error messages / suggestions).
    std::vector<const MethodDescriptor*> methodsFor(TypeKind kind) const;

    // Get all array methods.
    std::vector<const MethodDescriptor*> arrayMethods() const;

private:
    // Key: "TypeKind::methodName" → MethodDescriptor
    std::unordered_multimap<std::string, MethodDescriptor> methods_;

    // Key: methodName → MethodDescriptor (for array methods)
    std::unordered_map<std::string, MethodDescriptor> arrayMethods_;

    void registerMethod(MethodDescriptor desc);
    void registerArrayMethod(MethodDescriptor desc);
    void registerBuiltins();

    // Helpers to build the key
    static std::string makeKey(TypeKind kind, const std::string& name);
    static std::string kindToString(TypeKind kind);
};
