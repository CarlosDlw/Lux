#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <initializer_list>

// Describes a single builtin function signature for the checker.
// Types use lux-level names: "int32", "int64", "float64", "string", "bool",
// "char", "void", "usize", "*void", etc.
// Special placeholders:
//   "_any"     — accepts any single type (polymorphic arg)
//   "_numeric" — accepts any numeric type (int or float)
//   "_integer" — accepts any integer type
//   "_float"   — accepts any float type

struct BuiltinSignature {
    std::string              name;
    std::string              returnType;  // lux type name
    std::vector<std::string> paramTypes;  // lux type names (fixed params only)
    bool isPolymorphic = false; // return type depends on argument type
    bool isVariadic    = false; // accepts extra args beyond paramTypes
};

// Central registry of all stdlib function signatures and constant types.
// Used by the Checker to validate call argument count/types and resolve
// the type of imported constants (e.g. PI, INT32_MAX).
class BuiltinRegistry {
public:
    BuiltinRegistry();

    // Look up a function by its lux-level name (e.g. "println", "sqrt").
    // Returns nullptr if not found.
    const BuiltinSignature* lookup(const std::string& name) const;

    // Look up a constant's type by its name (e.g. "PI" → "float64").
    // Returns empty string if not a known constant.
    const std::string& lookupConstant(const std::string& name) const;

private:
    void add(std::string name, std::string returnType,
             std::vector<std::string> params, bool poly = false,
             bool variadic = false);
    void addConstant(std::string name, std::string typeName);

    std::unordered_map<std::string, BuiltinSignature> signatures_;
    std::unordered_map<std::string, std::string> constants_;
};
