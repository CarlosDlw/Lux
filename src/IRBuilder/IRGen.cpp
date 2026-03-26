#include "IRBuilder/IRGen.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>

#include "generated/LuxLexer.h"
#include "namespace/NamespaceRegistry.h"
#include "ffi/CBindings.h"

#include <iostream>

void IRGen::setNamespaceContext(const NamespaceRegistry* registry,
                                 const std::string& currentNamespace,
                                 const std::string& currentFile) {
    nsRegistry_       = registry;
    currentNamespace_ = currentNamespace;
    currentFile_      = currentFile;
}

void IRGen::setCBindings(const CBindings* bindings) {
    cBindings_ = bindings;
}

llvm::Function* IRGen::declareCFunction(const std::string& name) {
    // Already declared in this module?
    if (auto* fn = module_->getFunction(name))
        return fn;

    if (!cBindings_) return nullptr;

    auto* cfunc = cBindings_->findFunction(name);
    if (!cfunc) return nullptr;

    auto& dl = module_->getDataLayout();

    // Build original LLVM types
    auto* origRetTy = cfunc->returnType->toLLVMType(*context_, dl);
    std::vector<llvm::Type*> origParamTypes;
    for (auto* pTI : cfunc->paramTypes)
        origParamTypes.push_back(pTI->toLLVMType(*context_, dl));

    // Classify return and params for x86-64 ABI coercion
    CABIInfo abiInfo;
    abiInfo.returnInfo = ABIClassifier::classifyReturn(origRetTy, dl, *context_);
    abiInfo.originalRetType = origRetTy;

    for (auto* pty : origParamTypes) {
        abiInfo.paramInfos.push_back(
            ABIClassifier::classifyParam(pty, dl, *context_));
        abiInfo.originalParamTypes.push_back(pty);
    }

    // Build coerced function type
    llvm::Type* coercedRetTy = origRetTy;
    if (abiInfo.returnInfo.kind == ABIArgKind::Coerce)
        coercedRetTy = abiInfo.returnInfo.coercedType;
    else if (abiInfo.returnInfo.kind == ABIArgKind::Indirect)
        coercedRetTy = llvm::Type::getVoidTy(*context_);

    std::vector<llvm::Type*> coercedParamTypes;

    // Sret: prepend hidden pointer parameter for large struct return
    if (abiInfo.returnInfo.kind == ABIArgKind::Indirect)
        coercedParamTypes.push_back(llvm::PointerType::getUnqual(*context_));

    for (size_t i = 0; i < origParamTypes.size(); i++) {
        auto& pi = abiInfo.paramInfos[i];
        switch (pi.kind) {
        case ABIArgKind::Direct:
            coercedParamTypes.push_back(origParamTypes[i]);
            break;
        case ABIArgKind::Coerce:
            coercedParamTypes.push_back(pi.coercedType);
            break;
        case ABIArgKind::Expand:
            for (auto* t : pi.expandedTypes)
                coercedParamTypes.push_back(t);
            break;
        case ABIArgKind::Indirect:
            coercedParamTypes.push_back(llvm::PointerType::getUnqual(*context_));
            break;
        }
    }

    auto* fnType = llvm::FunctionType::get(coercedRetTy, coercedParamTypes,
                                           cfunc->isVariadic);
    auto* fn = llvm::Function::Create(
        fnType, llvm::Function::ExternalLinkage, name, module_);

    // Add ABI attributes
    unsigned argIdx = 0;
    if (abiInfo.returnInfo.kind == ABIArgKind::Indirect) {
        fn->addParamAttr(0, llvm::Attribute::getWithStructRetType(
            *context_, origRetTy));
        fn->addParamAttr(0, llvm::Attribute::get(
            *context_, llvm::Attribute::NoAlias));
        fn->addParamAttr(0, llvm::Attribute::get(
            *context_, llvm::Attribute::Writable));
        argIdx = 1;
    }

    for (size_t i = 0; i < abiInfo.paramInfos.size(); i++) {
        if (abiInfo.paramInfos[i].kind == ABIArgKind::Indirect) {
            fn->addParamAttr(argIdx, llvm::Attribute::getWithByValType(
                *context_, origParamTypes[i]));
        }
        if (abiInfo.paramInfos[i].kind == ABIArgKind::Expand)
            argIdx += abiInfo.paramInfos[i].expandedTypes.size();
        else
            argIdx++;
    }

    // Store ABI info for call-site coercion
    cabiInfos_[name] = std::move(abiInfo);

    externCFunctions_.insert(name);
    return fn;
}

std::unique_ptr<IRModule> IRGen::generate(LuxParser::ProgramContext* tree,
                                          const std::string& moduleName) {
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto mod = std::make_unique<llvm::Module>(moduleName, *ctx);
    llvm::IRBuilder<> builder(*ctx);

    context_ = ctx.get();
    module_  = mod.get();
    builder_ = &builder;
    locals_.clear();
    imports_ = ImportResolver{};
    userImports_.clear();
    callTargetMap_.clear();
    externCFunctions_.clear();
    globalBuiltins_ = {"exit", "panic", "assert", "assertMsg",
                        "unreachable", "toInt", "toFloat", "toBool", "toString",
                        "cstr", "fromCStr", "fromCStrLen"};

    visitProgram(tree);

    // Validate the generated IR before handing it off
    std::string verifyErr;
    llvm::raw_string_ostream errStream(verifyErr);
    if (llvm::verifyModule(*module_, &errStream)) {
        std::cerr << "lux: IR verification failed:\n" << errStream.str() << "\n";
        context_ = nullptr;
        module_  = nullptr;
        builder_ = nullptr;
        return nullptr;
    }

    context_ = nullptr;
    module_  = nullptr;
    builder_ = nullptr;

    return std::make_unique<IRModule>(std::move(ctx), std::move(mod));
}

// ── Visitors ────────────────────────────────────────────────────────────────

std::any IRGen::visitProgram(LuxParser::ProgramContext* ctx) {
    // ── Register built-in Error struct type in LLVM ─────────────────────
    {
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* i32Ty   = llvm::Type::getInt32Ty(*context_);
        auto* strTy   = llvm::StructType::get(*context_, { ptrTy, usizeTy });
        auto* errorTy = llvm::StructType::create(
            *context_,
            { strTy, strTy, i32Ty, i32Ty },
            "Error");
        (void)errorTy;
    }

    // ── Register C struct types from #include bindings ────────────────
    if (cBindings_) {
        // Phase 1: Create all C structs as opaque LLVM types (forward decls)
        // This ensures all types exist before resolving inter-struct references.
        for (auto& [name, cstruct] : cBindings_->structs()) {
            if (llvm::StructType::getTypeByName(*context_, name))
                continue;
            llvm::StructType::create(*context_, name);
        }

        // Phase 2a: Set bodies for opaque types (unions, bitfield structs)
        // These use a byte-array representation and must be sized first so
        // that structs referencing them can be properly laid out.
        for (auto& [name, cstruct] : cBindings_->structs()) {
            if (!cstruct.fields.empty()) continue;

            auto* structTy = llvm::StructType::getTypeByName(*context_, name);
            if (!structTy || !structTy->isOpaque()) continue;

            if (cstruct.byteSize > 0) {
                auto* i8Ty = llvm::Type::getInt8Ty(*context_);
                auto* arrTy = llvm::ArrayType::get(i8Ty, cstruct.byteSize);
                structTy->setBody({ arrTy });
            }
        }

        // Phase 2b: Set bodies for structs with resolved fields
        for (auto& [name, cstruct] : cBindings_->structs()) {
            if (cstruct.fields.empty()) continue;

            auto* structTy = llvm::StructType::getTypeByName(*context_, name);
            if (!structTy || !structTy->isOpaque()) continue;

            std::vector<llvm::Type*> fieldTypes;
            bool allResolved = true;
            for (auto& field : cstruct.fields) {
                auto* llTy = field.typeInfo->toLLVMType(*context_,
                                module_->getDataLayout());
                if (!llTy) { allResolved = false; break; }
                fieldTypes.push_back(llTy);
            }
            if (!allResolved) continue;

            structTy->setBody(fieldTypes);

            // Also register in our TypeRegistry so resolveTypeInfo works
            TypeInfo ti;
            ti.name = name;
            ti.kind = TypeKind::Struct;
            ti.bitWidth = 0;
            ti.isSigned = false;
            ti.fields = cstruct.fields;
            typeRegistry_.registerType(std::move(ti));
        }

        // Register C enum constants as globals
        for (auto& [name, cenum] : cBindings_->enums()) {
            for (auto& [vname, val] : cenum.values) {
                cEnumConstants_[vname] = val;
            }
        }

        // Register C #define integer constants
        for (auto& [name, cmacro] : cBindings_->macros()) {
            cEnumConstants_[name] = cmacro.value;
        }

        // Register C #define struct literal macros (e.g. RAYWHITE)
        for (auto& [name, sm] : cBindings_->structMacros()) {
            cStructMacros_[name] = &sm;
        }

        // Declare C global variables as extern globals
        for (auto& [name, gvar] : cBindings_->globals()) {
            auto* llTy = gvar.type->toLLVMType(*context_,
                             module_->getDataLayout());
            if (!llTy) continue;

            auto* gv = new llvm::GlobalVariable(
                *module_, llTy, false,
                llvm::GlobalValue::ExternalLinkage,
                nullptr, name);
            cGlobals_[name] = { gv, gvar.type };
        }

        // Register C typedefs as type aliases
        for (auto& [name, ctdef] : cBindings_->typedefs()) {
            if (!typeRegistry_.lookup(name)) {
                TypeInfo ti = *ctdef.underlying;
                ti.name = name;
                typeRegistry_.registerType(std::move(ti));
            }
        }
    }

    // Process `use` declarations first so all imports are known
    for (auto* useDecl : ctx->useDecl()) {
        visit(useDecl);
    }

    // Register cross-file symbols (extern declarations, types, etc.)
    registerCrossFileSymbols(ctx);

    // Register type aliases first
    for (auto* decl : ctx->topLevelDecl()) {
        if (decl->typeAliasDecl())
            visit(decl);
    }
    // Register struct, union, and enum types first
    for (auto* decl : ctx->topLevelDecl()) {
        if (decl->structDecl() || decl->unionDecl() || decl->enumDecl())
            visit(decl);
    }
    // Register struct methods via `extend` blocks
    for (auto* decl : ctx->topLevelDecl()) {
        if (decl->extendDecl())
            visit(decl);
    }
    // Register extern (FFI) function declarations
    for (auto* decl : ctx->topLevelDecl()) {
        if (decl->externDecl())
            visit(decl);
    }
    // Forward-declare all user functions (signatures only)
    for (auto* decl : ctx->topLevelDecl()) {
        if (decl->functionDecl())
            forwardDeclareFunction(decl->functionDecl());
    }
    // Then process functions (generate bodies)
    for (auto* decl : ctx->topLevelDecl()) {
        if (decl->functionDecl())
            visit(decl);
    }
    return {};
}

std::any IRGen::visitStructDecl(LuxParser::StructDeclContext* ctx) {
    auto structName = ctx->IDENTIFIER()->getText();

    // Create opaque LLVM struct first (enables self-referencing pointer fields)
    auto* structType = llvm::StructType::create(*context_, structName);

    // Register skeleton in TypeRegistry so resolveTypeInfo can find it
    TypeInfo ti;
    ti.name = structName;
    ti.kind = TypeKind::Struct;
    ti.bitWidth = 0;
    ti.isSigned = false;
    typeRegistry_.registerType(ti);

    // Collect field types
    std::vector<llvm::Type*> fieldTypes;
    for (auto* field : ctx->structField()) {
        auto* fieldTI = resolveTypeInfo(field->typeSpec());
        fieldTypes.push_back(fieldTI->toLLVMType(*context_, module_->getDataLayout()));
        ti.fields.push_back({ field->IDENTIFIER()->getText(), fieldTI });
    }

    // Set the struct body now that all fields are resolved
    structType->setBody(fieldTypes);

    // Update registry with full type info (fields populated)
    typeRegistry_.registerType(std::move(ti));
    return {};
}

std::any IRGen::visitUnionDecl(LuxParser::UnionDeclContext* ctx) {
    auto unionName = ctx->IDENTIFIER()->getText();

    // Collect field types and compute the max field size
    std::vector<FieldInfo> fieldInfos;
    uint64_t maxSize = 0;

    for (auto* field : ctx->unionField()) {
        auto* fieldTI = resolveTypeInfo(field->typeSpec());
        auto* fieldLLTy = fieldTI->toLLVMType(*context_, module_->getDataLayout());
        uint64_t fieldSize = module_->getDataLayout().getTypeAllocSize(fieldLLTy);
        if (fieldSize > maxSize) maxSize = fieldSize;
        fieldInfos.push_back({ field->IDENTIFIER()->getText(), fieldTI });
    }

    if (maxSize == 0) maxSize = 1;

    // Create LLVM struct with a single [maxSize x i8] body
    auto* i8Ty = llvm::Type::getInt8Ty(*context_);
    auto* bodyTy = llvm::ArrayType::get(i8Ty, maxSize);
    auto* unionType = llvm::StructType::create(*context_, { bodyTy }, unionName);
    (void)unionType;

    // Register in TypeRegistry
    TypeInfo ti;
    ti.name = unionName;
    ti.kind = TypeKind::Union;
    ti.bitWidth = 0;
    ti.isSigned = false;
    ti.fields = std::move(fieldInfos);
    typeRegistry_.registerType(std::move(ti));

    return {};
}

std::any IRGen::visitEnumDecl(LuxParser::EnumDeclContext* ctx) {
    auto identifiers = ctx->IDENTIFIER();
    auto enumName = identifiers[0]->getText();

    TypeInfo ti;
    ti.name = enumName;
    ti.kind = TypeKind::Enum;
    ti.bitWidth = 32;
    ti.isSigned = false;
    ti.builtinSuffix = "i32";

    for (size_t i = 1; i < identifiers.size(); i++) {
        ti.enumVariants.push_back(identifiers[i]->getText());
    }

    typeRegistry_.registerType(std::move(ti));
    return {};
}

// ═══════════════════════════════════════════════════════════════════════
//  extend StructName { methods... }
// ═══════════════════════════════════════════════════════════════════════

std::any IRGen::visitExtendDecl(LuxParser::ExtendDeclContext* ctx) {
    auto structName = ctx->IDENTIFIER()->getText();
    auto* structTI = typeRegistry_.lookup(structName);
    if (!structTI || structTI->kind != TypeKind::Struct) {
        std::cerr << "lux: cannot extend unknown struct '" << structName << "'\n";
        return {};
    }

    auto* structLLTy = structTI->toLLVMType(*context_, module_->getDataLayout());
    auto* ptrTy = llvm::PointerType::getUnqual(*context_);

    for (auto* method : ctx->extendMethod()) {
        auto methodName = method->IDENTIFIER(0)->getText();
        auto* retTI = resolveTypeInfo(method->typeSpec());
        auto* retLLTy = retTI->toLLVMType(*context_, module_->getDataLayout());

        bool isStatic = (method->AMPERSAND() == nullptr);

        // Collect parameter types and TypeInfos
        std::vector<llvm::Type*> paramLLTypes;
        std::vector<const TypeInfo*> paramTIs;

        if (!isStatic) {
            paramLLTypes.push_back(ptrTy); // &self as first param
        }

        // Get param list from the correct source
        std::vector<LuxParser::ParamContext*> params;
        if (isStatic) {
            if (auto* pl = method->paramList())
                params = pl->param();
        } else {
            params = method->param();
        }

        for (auto* param : params) {
            auto* pTI = resolveTypeInfo(param->typeSpec());
            paramTIs.push_back(pTI);
            paramLLTypes.push_back(pTI->toLLVMType(*context_, module_->getDataLayout()));
        }

        // Create the function: StructName__methodName
        auto funcName = structName + "__" + methodName;
        auto* fnType = llvm::FunctionType::get(retLLTy, paramLLTypes, false);
        auto* fn = llvm::Function::Create(
            fnType, llvm::Function::InternalLinkage, funcName, module_);

        // Register in the appropriate map
        if (isStatic)
            staticStructMethods_[structName][methodName] = fn;
        else
            structMethods_[structName][methodName] = fn;

        // Save state
        auto* savedFunc = currentFunction_;
        auto  savedLocals = locals_;
        auto* savedBB = builder_->GetInsertBlock();
        currentFunction_ = fn;
        locals_.clear();

        // Create entry block
        auto* entry = llvm::BasicBlock::Create(*context_, "entry", fn);
        builder_->SetInsertPoint(entry);

        if (!isStatic) {
            // Set up &self parameter
            auto* selfArg = fn->getArg(0);
            selfArg->setName("self");
            auto* selfAlloca = builder_->CreateAlloca(ptrTy, nullptr, "self");
            builder_->CreateStore(selfArg, selfAlloca);

            // Create pointer-to-struct TypeInfo for `self`
            auto ptrTIName = "*" + structName;
            if (!typeRegistry_.lookup(ptrTIName)) {
                TypeInfo ptrTI;
                ptrTI.name = ptrTIName;
                ptrTI.kind = TypeKind::Pointer;
                ptrTI.bitWidth = 0;
                ptrTI.isSigned = false;
                ptrTI.builtinSuffix = "ptr";
                ptrTI.pointeeType = structTI;
                typeRegistry_.registerType(std::move(ptrTI));
            }
            locals_["self"] = { selfAlloca, typeRegistry_.lookup(ptrTIName), 0 };
        }

        // Set up parameters
        size_t argOffset = isStatic ? 0 : 1;
        for (size_t i = 0; i < params.size(); i++) {
            auto paramName = params[i]->IDENTIFIER()->getText();
            auto* arg = fn->getArg(i + argOffset);
            arg->setName(paramName);
            auto* paramLLTy = paramLLTypes[i + argOffset];
            auto* alloca = builder_->CreateAlloca(paramLLTy, nullptr, paramName);
            builder_->CreateStore(arg, alloca);
            locals_[paramName] = { alloca, paramTIs[i], 0, /*isParam=*/true };
        }

        // Visit the method body statements directly
        for (auto* stmt : method->block()->statement()) {
            visit(stmt);
        }

        // If no terminator, add implicit return
        if (!builder_->GetInsertBlock()->getTerminator()) {
            if (retLLTy->isVoidTy())
                builder_->CreateRetVoid();
            else
                builder_->CreateRet(llvm::UndefValue::get(retLLTy));
        }

        // Restore state
        currentFunction_ = savedFunc;
        locals_ = savedLocals;
        if (savedBB)
            builder_->SetInsertPoint(savedBB);
    }

    return {};
}

// ── FFI: extern function declarations ─────────────────────────────────────────

std::any IRGen::visitExternDecl(LuxParser::ExternDeclContext* ctx) {
    auto funcName = ctx->IDENTIFIER()->getText();

    // Don't re-declare if already present
    if (module_->getFunction(funcName))
        return {};

    auto* retTI   = resolveTypeInfo(ctx->typeSpec());
    auto* retLLTy = retTI->toLLVMType(*context_, module_->getDataLayout());

    bool isVariadic = (ctx->SPREAD() != nullptr);

    std::vector<llvm::Type*> paramTypes;
    if (auto* paramList = ctx->externParamList()) {
        for (auto* param : paramList->externParam()) {
            auto* pTI   = resolveTypeInfo(param->typeSpec());
            auto* pLLTy = pTI->toLLVMType(*context_, module_->getDataLayout());
            paramTypes.push_back(pLLTy);
        }
    }

    auto* fnType = llvm::FunctionType::get(retLLTy, paramTypes, isVariadic);
    auto* fn = llvm::Function::Create(
        fnType, llvm::Function::ExternalLinkage, funcName, module_);

    // Mark as an extern C function for call resolution
    externCFunctions_.insert(funcName);

    return {};
}

std::any IRGen::visitTypeAliasDecl(LuxParser::TypeAliasDeclContext* ctx) {
    auto name = ctx->IDENTIFIER()->getText();

    // Skip if already registered (e.g., from a previous pass)
    if (typeRegistry_.lookup(name))
        return {};

    auto* typeSpecCtx = ctx->typeSpec();
    if (auto* fnSpec = typeSpecCtx->fnTypeSpec()) {
        TypeInfo ti;
        ti.name = name;
        ti.kind = TypeKind::Function;
        ti.bitWidth = 0;
        ti.isSigned = false;
        ti.builtinSuffix = "ptr";

        // All typeSpecs from fnTypeSpec: params are [0..n-2], return is [n-1]
        auto specs = fnSpec->typeSpec();
        ti.returnType = resolveTypeInfo(specs.back());

        for (size_t i = 0; i + 1 < specs.size(); i++) {
            ti.paramTypes.push_back(resolveTypeInfo(specs[i]));
        }

        typeRegistry_.registerType(std::move(ti));
    } else {
        // Simple alias: type MyInt = int32;
        auto* baseTI = resolveTypeInfo(typeSpecCtx);
        TypeInfo ti = *baseTI;
        ti.name = name;
        typeRegistry_.registerType(std::move(ti));
    }

    return {};
}

// ── Forward-declare a user function (signature only, no body) ───────────
void IRGen::forwardDeclareFunction(LuxParser::FunctionDeclContext* ctx) {
    auto* retInfo    = resolveTypeInfo(ctx->typeSpec());
    auto* returnType = retInfo->toLLVMType(*context_, module_->getDataLayout());
    auto  funcName   = ctx->IDENTIFIER()->getText();

    bool isMainWithArgs = (funcName == "main" && ctx->paramList() != nullptr);

    // Collect parameter types
    std::vector<llvm::Type*> paramTypes;
    int variadicIdx = -1;
    if (auto* params = ctx->paramList()) {
        auto paramList = params->param();
        for (size_t i = 0; i < paramList.size(); i++) {
            auto* param = paramList[i];
            auto* pInfo = resolveTypeInfo(param->typeSpec());
            if (param->SPREAD()) {
                variadicIdx = static_cast<int>(i);
                paramTypes.push_back(llvm::PointerType::getUnqual(*context_));
                paramTypes.push_back(llvm::Type::getInt64Ty(*context_));
            } else {
                paramTypes.push_back(pInfo->toLLVMType(*context_, module_->getDataLayout()));
            }
        }
    }

    std::string emitName;
    if (isMainWithArgs) {
        emitName = "lux_user_main";
    } else if (funcName == "main") {
        emitName = "main";
    } else if (nsRegistry_) {
        emitName = NamespaceRegistry::mangle(currentNamespace_, funcName);
    } else {
        emitName = funcName;
    }

    // Skip if already declared (e.g. from cross-file registration)
    if (module_->getFunction(emitName))
        return;

    auto* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, emitName, module_);

    // Register in callTargetMap_ so calls resolve before body generation
    if (nsRegistry_ && funcName != "main") {
        callTargetMap_[funcName] = emitName;
    }

    // Register variadic function info
    if (variadicIdx >= 0 && ctx->paramList()) {
        auto* vParam = ctx->paramList()->param(variadicIdx);
        auto* vInfo = resolveTypeInfo(vParam->typeSpec());
        variadicFunctions_[emitName] = {
            static_cast<size_t>(variadicIdx), vInfo
        };
    }
}

std::any IRGen::visitFunctionDecl(LuxParser::FunctionDeclContext* ctx) {
    auto* retInfo    = resolveTypeInfo(ctx->typeSpec());
    auto* returnType = retInfo->toLLVMType(*context_, module_->getDataLayout());
    auto  funcName   = ctx->IDENTIFIER()->getText();

    // ── Special handling: main(Vec<string> args) ────────────────────────
    // When main has parameters, we generate:
    //   1) lux_user_main(ptr %args) — the user's actual code
    //   2) main(i32 %argc, ptr %argv) — a C-ABI wrapper that builds the Vec
    bool isMainWithArgs = (funcName == "main" && ctx->paramList() != nullptr);

    // Collect parameter types (variadic param → ptr + i64 pair)
    std::vector<llvm::Type*> paramTypes;
    int variadicIdx = -1; // index of the variadic param in the grammar param list
    if (auto* params = ctx->paramList()) {
        auto paramList = params->param();
        for (size_t i = 0; i < paramList.size(); i++) {
            auto* param = paramList[i];
            auto* pInfo = resolveTypeInfo(param->typeSpec());
            if (param->SPREAD()) {
                // Variadic: emit as (ptr, i64) for the packed array
                variadicIdx = static_cast<int>(i);
                paramTypes.push_back(llvm::PointerType::getUnqual(*context_));
                paramTypes.push_back(llvm::Type::getInt64Ty(*context_));
            } else {
                paramTypes.push_back(pInfo->toLLVMType(*context_, module_->getDataLayout()));
            }
        }
    }

    // If main has args, emit the user's code under a different name
    // Apply namespace mangling for non-main functions
    std::string emitName;
    if (isMainWithArgs) {
        emitName = "lux_user_main";
    } else if (funcName == "main") {
        emitName = "main";
    } else if (nsRegistry_) {
        emitName = NamespaceRegistry::mangle(currentNamespace_, funcName);
    } else {
        emitName = funcName;
    }

    // Register in callTargetMap_ so local calls resolve to this mangled name
    if (nsRegistry_ && funcName != "main") {
        callTargetMap_[funcName] = emitName;
    }

    // Reuse forward-declared function, or create if not yet declared
    llvm::Function* func = module_->getFunction(emitName);
    if (!func) {
        auto* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
        func = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, emitName, module_);
    }

    // Name each parameter for readable IR
    if (auto* params = ctx->paramList()) {
        auto paramList = params->param();
        size_t llvmIdx = 0;
        for (size_t i = 0; i < paramList.size(); i++) {
            auto pName = paramList[i]->IDENTIFIER()->getText();
            if (paramList[i]->SPREAD()) {
                func->getArg(llvmIdx)->setName(pName + ".ptr");
                func->getArg(llvmIdx + 1)->setName(pName + ".len");
                llvmIdx += 2;
            } else {
                func->getArg(llvmIdx)->setName(pName);
                llvmIdx++;
            }
        }
    }

    currentFunction_ = func;
    locals_.clear(); // each function has its own scope
    variadicParams_.clear();
    deferStack_.clear();

    // Register variadic function info for call-site packing
    if (variadicIdx >= 0 && ctx->paramList()) {
        auto* vParam = ctx->paramList()->param(variadicIdx);
        auto* vInfo = resolveTypeInfo(vParam->typeSpec());
        variadicFunctions_[emitName] = {
            static_cast<size_t>(variadicIdx), vInfo
        };
    }

    // Create entry block first, then store params as allocas
    auto* entry = llvm::BasicBlock::Create(*context_, "entry", func);
    builder_->SetInsertPoint(entry);

    if (auto* params = ctx->paramList()) {
        auto paramList = params->param();
        size_t llvmIdx = 0;
        for (size_t i = 0; i < paramList.size(); i++) {
            auto* param = paramList[i];
            auto* pInfo = resolveTypeInfo(param->typeSpec());
            auto  pName = param->IDENTIFIER()->getText();

            if (param->SPREAD()) {
                // Variadic: store ptr and len in separate allocas
                auto* ptrTy = llvm::PointerType::getUnqual(*context_);
                auto* i64Ty = llvm::Type::getInt64Ty(*context_);

                auto* ptrAlloca = builder_->CreateAlloca(ptrTy, nullptr, pName + ".ptr");
                auto* lenAlloca = builder_->CreateAlloca(i64Ty, nullptr, pName + ".len");
                builder_->CreateStore(func->getArg(llvmIdx), ptrAlloca);
                builder_->CreateStore(func->getArg(llvmIdx + 1), lenAlloca);

                variadicParams_[pName] = { ptrAlloca, lenAlloca, pInfo };
                llvmIdx += 2;
            } else {
                auto* pType = pInfo->toLLVMType(*context_, module_->getDataLayout());
                auto* alloca = builder_->CreateAlloca(pType, nullptr, pName);
                builder_->CreateStore(func->getArg(llvmIdx), alloca);
                locals_[pName] = { alloca, pInfo, 0, /*isParam=*/true };
                llvmIdx++;
            }
        }
    }

    // Visit block body (statements only, skip block's own entry creation)
    for (auto* stmt : ctx->block()->statement()) {
        visit(stmt);
    }

    // If no terminator, emit deferred/auto cleanups and add implicit return
    if (!builder_->GetInsertBlock()->getTerminator()) {
        emitAllCleanups();
        if (func->getReturnType()->isVoidTy())
            builder_->CreateRetVoid();
        else
            builder_->CreateRet(llvm::UndefValue::get(func->getReturnType()));
    }

    currentFunction_ = nullptr;

    // ── Generate the real main() wrapper ────────────────────────────────
    if (isMainWithArgs) {
        auto* i32Ty   = llvm::Type::getInt32Ty(*context_);
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* voidTy  = llvm::Type::getVoidTy(*context_);
        auto* vecTy   = getOrCreateVecStructType();

        // main(i32 %argc, ptr %argv) → i32
        auto* wrapperType = llvm::FunctionType::get(i32Ty, {i32Ty, ptrTy}, false);
        auto* wrapperFn   = llvm::Function::Create(
            wrapperType, llvm::Function::ExternalLinkage, "main", module_);

        auto* wrapperEntry = llvm::BasicBlock::Create(*context_, "entry", wrapperFn);
        builder_->SetInsertPoint(wrapperEntry);

        auto* argc = wrapperFn->getArg(0);
        auto* argv = wrapperFn->getArg(1);
        argc->setName("argc");
        argv->setName("argv");

        // Alloca a Vec<string> on the stack
        auto* vecAlloca = builder_->CreateAlloca(vecTy, nullptr, "args");

        // Call lux_args_init(&args, argc, argv)
        auto argsInit = declareBuiltin("lux_args_init", voidTy,
                                       {ptrTy, i32Ty, ptrTy});
        builder_->CreateCall(argsInit, {vecAlloca, argc, argv});

        // Call lux_user_main(args_vec_struct)
        // The user function takes the vec struct by value (loaded from alloca)
        auto* vecVal = builder_->CreateLoad(vecTy, vecAlloca, "args_vec");
        auto* retVal = builder_->CreateCall(func, {vecVal}, "ret");

        // Free the vec
        auto vecFree = declareBuiltin("lux_vec_free_str", voidTy, {ptrTy});
        builder_->CreateCall(vecFree, {vecAlloca});

        builder_->CreateRet(retVal);
    }

    return {};
}

std::any IRGen::visitBlock(LuxParser::BlockContext* ctx) {
    auto* entry = llvm::BasicBlock::Create(*context_, "entry", currentFunction_);
    builder_->SetInsertPoint(entry);

    for (auto* stmt : ctx->statement()) {
        visit(stmt);
    }

    return {};
}

// use std::log::println;   →  register in ImportResolver
std::any IRGen::visitUseItem(LuxParser::UseItemContext* ctx) {
    std::string path;
    for (auto* id : ctx->modulePath()->IDENTIFIER()) {
        if (!path.empty()) path += "::";
        path += id->getText();
    }
    auto symbolName = ctx->IDENTIFIER()->getText();

    if (NamespaceRegistry::isStdModule(path)) {
        imports_.addImport(path, symbolName);
    } else {
        // User namespace import — record for call resolution
        userImports_[symbolName] = path;
    }
    return {};
}

// use std::log::{ println, print };
std::any IRGen::visitUseGroup(LuxParser::UseGroupContext* ctx) {
    std::string path;
    for (auto* id : ctx->modulePath()->IDENTIFIER()) {
        if (!path.empty()) path += "::";
        path += id->getText();
    }

    if (NamespaceRegistry::isStdModule(path)) {
        for (auto* id : ctx->IDENTIFIER()) {
            imports_.addImport(path, id->getText());
        }
    } else {
        for (auto* id : ctx->IDENTIFIER()) {
            userImports_[id->getText()] = path;
        }
    }
    return {};
}

// ═══════════════════════════════════════════════════════════════════════
//  Cross-file symbol registration (namespace support)
// ═══════════════════════════════════════════════════════════════════════

void IRGen::registerCrossFileSymbols(LuxParser::ProgramContext* /*ctx*/) {
    if (!nsRegistry_) return;

    // ── Phase 1: Register struct / enum / typeAlias types from other files
    //             in the same namespace ──────────────────────────────────
    auto sameNsSymbols = nsRegistry_->getExternalSymbols(
        currentNamespace_, currentFile_);

    for (auto* sym : sameNsSymbols) {
        if (sym->kind == ExportedSymbol::Struct) {
            auto* structCtx = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl);
            if (structCtx && !typeRegistry_.lookup(sym->name))
                visitStructDecl(structCtx);
        } else if (sym->kind == ExportedSymbol::Union) {
            auto* unionCtx = dynamic_cast<LuxParser::UnionDeclContext*>(sym->decl);
            if (unionCtx && !typeRegistry_.lookup(sym->name))
                visitUnionDecl(unionCtx);
        } else if (sym->kind == ExportedSymbol::Enum) {
            auto* enumCtx = dynamic_cast<LuxParser::EnumDeclContext*>(sym->decl);
            if (enumCtx && !typeRegistry_.lookup(sym->name))
                visitEnumDecl(enumCtx);
        } else if (sym->kind == ExportedSymbol::TypeAlias) {
            auto* aliasCtx = dynamic_cast<LuxParser::TypeAliasDeclContext*>(sym->decl);
            if (aliasCtx && !typeRegistry_.lookup(sym->name))
                visitTypeAliasDecl(aliasCtx);
        }
    }

    // ── Phase 1b: Register types from explicitly imported namespaces ────
    for (auto& [symbolName, sourceNs] : userImports_) {
        auto* sym = nsRegistry_->findSymbol(sourceNs, symbolName);
        if (!sym) continue;

        if (sym->kind == ExportedSymbol::Struct) {
            auto* structCtx = dynamic_cast<LuxParser::StructDeclContext*>(sym->decl);
            if (structCtx && !typeRegistry_.lookup(sym->name))
                visitStructDecl(structCtx);
        } else if (sym->kind == ExportedSymbol::Union) {
            auto* unionCtx = dynamic_cast<LuxParser::UnionDeclContext*>(sym->decl);
            if (unionCtx && !typeRegistry_.lookup(sym->name))
                visitUnionDecl(unionCtx);
        } else if (sym->kind == ExportedSymbol::Enum) {
            auto* enumCtx = dynamic_cast<LuxParser::EnumDeclContext*>(sym->decl);
            if (enumCtx && !typeRegistry_.lookup(sym->name))
                visitEnumDecl(enumCtx);
        } else if (sym->kind == ExportedSymbol::TypeAlias) {
            auto* aliasCtx = dynamic_cast<LuxParser::TypeAliasDeclContext*>(sym->decl);
            if (aliasCtx && !typeRegistry_.lookup(sym->name))
                visitTypeAliasDecl(aliasCtx);
        }
    }

    // ── Phase 2: Declare extern functions from same namespace ──────────
    for (auto* sym : sameNsSymbols) {
        if (sym->kind == ExportedSymbol::Function) {
            auto mangledName = NamespaceRegistry::mangle(
                currentNamespace_, sym->name);
            auto* funcCtx = dynamic_cast<LuxParser::FunctionDeclContext*>(sym->decl);
            if (funcCtx && !module_->getFunction(mangledName)) {
                declareExternFunction(mangledName, funcCtx);
            }
            // Map unmangled name → mangled name for call resolution
            callTargetMap_[sym->name] = mangledName;
        }
    }

    // ── Phase 2b: Declare extern functions from imported namespaces ────
    for (auto& [symbolName, sourceNs] : userImports_) {
        auto* sym = nsRegistry_->findSymbol(sourceNs, symbolName);
        if (!sym || sym->kind != ExportedSymbol::Function) continue;

        auto mangledName = NamespaceRegistry::mangle(sourceNs, sym->name);
        auto* funcCtx = dynamic_cast<LuxParser::FunctionDeclContext*>(sym->decl);
        if (funcCtx && !module_->getFunction(mangledName)) {
            declareExternFunction(mangledName, funcCtx);
        }
        callTargetMap_[symbolName] = mangledName;
    }

    // ── Phase 3: Register local functions into callTargetMap_ ──────────
    //    (done later in visitFunctionDecl via mangling)
}

void IRGen::declareExternFunction(const std::string& mangledName,
                                   LuxParser::FunctionDeclContext* decl) {
    auto* retTI     = resolveTypeInfo(decl->typeSpec());
    auto* retLLType = retTI->toLLVMType(*context_, module_->getDataLayout());

    std::vector<llvm::Type*> paramTypes;
    int variadicIdx = -1;

    if (auto* params = decl->paramList()) {
        auto paramList = params->param();
        for (size_t i = 0; i < paramList.size(); i++) {
            auto* param = paramList[i];
            auto* pTI = resolveTypeInfo(param->typeSpec());
            if (param->SPREAD()) {
                variadicIdx = static_cast<int>(i);
                paramTypes.push_back(llvm::PointerType::getUnqual(*context_));
                paramTypes.push_back(llvm::Type::getInt64Ty(*context_));
            } else {
                paramTypes.push_back(pTI->toLLVMType(*context_, module_->getDataLayout()));
            }
        }
    }

    auto* fnType = llvm::FunctionType::get(retLLType, paramTypes, false);
    llvm::Function::Create(fnType, llvm::Function::ExternalLinkage,
                           mangledName, module_);

    // Register variadic info under the mangled name
    if (variadicIdx >= 0 && decl->paramList()) {
        auto* vParam = decl->paramList()->param(variadicIdx);
        auto* vInfo = resolveTypeInfo(vParam->typeSpec());
        variadicFunctions_[mangledName] = {
            static_cast<size_t>(variadicIdx), vInfo
        };
    }
}

std::string IRGen::resolveCallTarget(const std::string& name) const {
    auto it = callTargetMap_.find(name);
    if (it != callTargetMap_.end())
        return it->second;
    return name;
}

// int32 x = 42;   or   []int32 arr = [1, 2, 3];   or   Vec<int32> v = [1, 2, 3];
std::any IRGen::visitVarDeclStmt(LuxParser::VarDeclStmtContext* ctx) {
    auto* ti   = resolveTypeInfo(ctx->typeSpec());
    auto  dims = countArrayDims(ctx->typeSpec());
    auto  name = ctx->IDENTIFIER()->getText();

    // Extended type (Vec<T>, Map<K,V>, Task<T>, Mutex) initialization
    if (ti->kind == TypeKind::Extended) {
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);

        // Task<T> — initialized from spawn expression
        if (ti->extendedKind == "Task") {
            auto* alloca = builder_->CreateAlloca(ptrTy, nullptr, name);
            locals_[name] = { alloca, ti, 0 };
            if (ctx->expression()) {
                auto  initVal = visit(ctx->expression());
                auto* val     = std::any_cast<llvm::Value*>(initVal);
                builder_->CreateStore(val, alloca);
            } else {
                builder_->CreateStore(
                    llvm::ConstantPointerNull::get(ptrTy), alloca);
            }
            return {};
        }

        // Mutex — initialized via lux_mutexCreate()
        if (ti->extendedKind == "Mutex") {
            auto* alloca = builder_->CreateAlloca(ptrTy, nullptr, name);
            locals_[name] = { alloca, ti, 0 };
            auto callee = declareBuiltin("lux_mutexCreate", ptrTy, {});
            auto* mtx = builder_->CreateCall(callee, {}, "mutex");
            builder_->CreateStore(mtx, alloca);
            return {};
        }

        if (ti->extendedKind == "Map") {
            // Map<K,V> — always init empty
            auto* mapTy  = getOrCreateMapStructType();
            auto  suffix  = ti->builtinSuffix;
            auto* alloca = builder_->CreateAlloca(mapTy, nullptr, name);
            locals_[name] = { alloca, ti, 0 };

            auto initFn = declareBuiltin(
                "lux_map_init_" + suffix,
                llvm::Type::getVoidTy(*context_),
                { ptrTy });
            builder_->CreateCall(initFn, { alloca });
            return {};
        }

        if (ti->extendedKind == "Set") {
            // Set<T> — always init empty
            auto* setTy  = getOrCreateSetStructType();
            auto  suffix = ti->elementType->builtinSuffix;
            auto* alloca = builder_->CreateAlloca(setTy, nullptr, name);
            locals_[name] = { alloca, ti, 0 };

            auto initFn = declareBuiltin(
                "lux_set_init_" + suffix,
                llvm::Type::getVoidTy(*context_),
                { ptrTy });
            builder_->CreateCall(initFn, { alloca });
            return {};
        }

        // Vec<T> initialized from array literal or function call
        auto* vecTy   = getOrCreateVecStructType();
        auto  suffix  = getVecSuffix(ti->elementType);

        auto* alloca = builder_->CreateAlloca(vecTy, nullptr, name);
        locals_[name] = { alloca, ti, 0 };

        // No initializer — default to empty vec
        if (!ctx->expression()) {
            auto initFn = declareBuiltin(
                "lux_vec_init_" + suffix,
                llvm::Type::getVoidTy(*context_),
                { ptrTy });
            builder_->CreateCall(initFn, { alloca });
            return {};
        }

        // Check if the initializer is an array literal
        auto* arrLit = dynamic_cast<LuxParser::ArrayLitExprContext*>(
                ctx->expression());

        if (arrLit) {
            // Collect element values
            std::vector<llvm::Value*> vals;
            for (auto* e : arrLit->expression())
                vals.push_back(std::any_cast<llvm::Value*>(visit(e)));

            auto* elemLLTy = ti->elementType->toLLVMType(
                *context_, module_->getDataLayout());

            if (vals.empty()) {
                // Empty vec: call init
                auto initFn = declareBuiltin(
                    "lux_vec_init_" + suffix,
                    llvm::Type::getVoidTy(*context_),
                    { ptrTy });
                builder_->CreateCall(initFn, { alloca });
            } else {
                // Non-empty vec: init with capacity, then push each
                auto initCapFn = declareBuiltin(
                    "lux_vec_init_cap_" + suffix,
                    llvm::Type::getVoidTy(*context_),
                    { ptrTy, usizeTy });
                builder_->CreateCall(initCapFn, {
                    alloca,
                    llvm::ConstantInt::get(usizeTy, vals.size())
                });

                auto pushFn = declareBuiltin(
                    "lux_vec_push_" + suffix,
                    llvm::Type::getVoidTy(*context_),
                    { ptrTy, elemLLTy });
                for (auto* v : vals) {
                    if (v->getType() != elemLLTy) {
                        if (v->getType()->isIntegerTy() && elemLLTy->isIntegerTy())
                            v = builder_->CreateIntCast(v, elemLLTy,
                                                        ti->elementType->isSigned);
                    }
                    builder_->CreateCall(pushFn, { alloca, v });
                }
            }
        } else {
            // Function call or other expression returning Vec struct
            auto initVal = visit(ctx->expression());
            auto* val = std::any_cast<llvm::Value*>(initVal);
            builder_->CreateStore(val, alloca);
        }

        return {};
    }

    if (dims > 0) {
        // Array variable initialization
        auto* elemType = ti->toLLVMType(*context_, module_->getDataLayout());

        if (!ctx->expression()) {
            // No initializer — zero-initialize the array
            // Walk typeSpec to collect fixed-size dimensions [N]
            auto* spec = ctx->typeSpec();
            std::vector<unsigned> sizes;
            while (!spec->typeSpec().empty() && spec->INT_LIT()) {
                sizes.push_back(std::stoul(spec->INT_LIT()->getText()));
                spec = spec->typeSpec(0);
            }
            // Build nested array type from innermost to outermost
            llvm::Type* arrTy = elemType;
            for (auto it = sizes.rbegin(); it != sizes.rend(); ++it)
                arrTy = llvm::ArrayType::get(arrTy, *it);
            auto* alloca = builder_->CreateAlloca(arrTy, nullptr, name);
            builder_->CreateStore(llvm::Constant::getNullValue(arrTy), alloca);
            locals_[name] = { alloca, ti, dims };
            return {};
        }

        auto  initVal = visit(ctx->expression());
        auto* val     = std::any_cast<llvm::Value*>(initVal);

        auto* targetTy = buildTargetArrayType(val->getType(), elemType);
        auto* alloca   = builder_->CreateAlloca(targetTy, nullptr, name);
        storeArrayElements(val, alloca, targetTy, ti, dims);

        locals_[name] = { alloca, ti, dims };
    } else {
        // Scalar variable (existing flow)
        auto* type   = ti->toLLVMType(*context_, module_->getDataLayout());
        auto* alloca = builder_->CreateAlloca(type, nullptr, name);
        locals_[name] = { alloca, ti, 0 };

        if (!ctx->expression()) {
            // No initializer — zero-initialize
            builder_->CreateStore(llvm::Constant::getNullValue(type), alloca);
            return {};
        }

        auto  initVal = visit(ctx->expression());
        auto* val     = std::any_cast<llvm::Value*>(initVal);

        if (val->getType() != type) {
            if (val->getType()->isIntegerTy() && type->isIntegerTy()) {
                val = builder_->CreateIntCast(val, type, ti->isSigned, name + "_cast");
            } else if (val->getType()->isFloatingPointTy() && type->isFloatingPointTy()) {
                if (val->getType()->getPrimitiveSizeInBits() > type->getPrimitiveSizeInBits())
                    val = builder_->CreateFPTrunc(val, type, name + "_trunc");
                else
                    val = builder_->CreateFPExt(val, type, name + "_ext");
            }
        }

        builder_->CreateStore(val, alloca);
    }

    return {};
}

// x = 42;  or  arr[i] = val;   arr[i][j] = val;
std::any IRGen::visitAssignStmt(LuxParser::AssignStmtContext* ctx) {
    auto varName = ctx->IDENTIFIER()->getText();
    auto it = locals_.find(varName);

    // ── C global variable assignment ────────────────────────────────
    if (it == locals_.end()) {
        auto git = cGlobals_.find(varName);
        if (git != cGlobals_.end()) {
            auto& entry = git->second;
            auto exprs = ctx->expression();
            if (exprs.size() == 1) {
                auto* val = std::any_cast<llvm::Value*>(visit(exprs[0]));
                auto* gvTy = entry.global->getValueType();
                if (val->getType() != gvTy) {
                    if (val->getType()->isIntegerTy() && gvTy->isIntegerTy())
                        val = builder_->CreateIntCast(val, gvTy,
                                  entry.type->isSigned);
                }
                builder_->CreateStore(val, entry.global);
            }
            return {};
        }
        std::cerr << "lux: undefined variable '" << varName << "'\n";
        return {};
    }

    auto* alloca    = it->second.alloca;
    auto* allocType = alloca->getAllocatedType();
    auto* elemTI    = it->second.typeInfo;

    auto exprs = ctx->expression();

    // Simple reassignment: x = 42; (no bracket expressions)
    if (exprs.size() == 1) {
        auto* val = std::any_cast<llvm::Value*>(visit(exprs[0]));
        auto* varTy = allocType;

        if (val->getType() != varTy) {
            if (val->getType()->isIntegerTy() && varTy->isIntegerTy())
                val = builder_->CreateIntCast(val, varTy, elemTI->isSigned);
            else if (val->getType()->isFloatingPointTy() && varTy->isFloatingPointTy()) {
                if (val->getType()->getPrimitiveSizeInBits() > varTy->getPrimitiveSizeInBits())
                    val = builder_->CreateFPTrunc(val, varTy);
                else
                    val = builder_->CreateFPExt(val, varTy);
            }
        }

        builder_->CreateStore(val, alloca);
        return {};
    }

    // Indexed assignment: arr[i] = val  or  vec[i] = val
    // All expressions except the last are indices; the last is the RHS value

    // ── Extended type (Vec<T>, Map<K,V>) index assignment ──────────────
    if (elemTI && elemTI->kind == TypeKind::Extended) {
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);

        if (elemTI->extendedKind == "Map") {
            // Map<K,V> subscript assignment: m[key] = val → lux_map_set_<suffix>
            auto* keyLLTy = elemTI->keyType->toLLVMType(*context_, module_->getDataLayout());
            auto* valLLTy = elemTI->valueType->toLLVMType(*context_, module_->getDataLayout());
            auto suffix = elemTI->builtinSuffix;

            auto* keyVal = std::any_cast<llvm::Value*>(visit(exprs[0]));
            if (keyVal->getType() != keyLLTy) {
                if (keyVal->getType()->isIntegerTy() && keyLLTy->isIntegerTy())
                    keyVal = builder_->CreateIntCast(keyVal, keyLLTy, elemTI->keyType->isSigned);
            }

            auto* val = std::any_cast<llvm::Value*>(visit(exprs.back()));
            if (val->getType() != valLLTy) {
                if (val->getType()->isIntegerTy() && valLLTy->isIntegerTy())
                    val = builder_->CreateIntCast(val, valLLTy, elemTI->valueType->isSigned);
                else if (val->getType()->isFloatingPointTy() && valLLTy->isFloatingPointTy()) {
                    if (val->getType()->getPrimitiveSizeInBits() > valLLTy->getPrimitiveSizeInBits())
                        val = builder_->CreateFPTrunc(val, valLLTy);
                    else
                        val = builder_->CreateFPExt(val, valLLTy);
                }
            }

            auto callee = declareBuiltin(
                "lux_map_set_" + suffix,
                llvm::Type::getVoidTy(*context_),
                { ptrTy, keyLLTy, valLLTy });
            builder_->CreateCall(callee, { alloca, keyVal, val });
            return {};
        }

        // Vec<T> index assignment: v[i] = x → lux_vec_set_<suffix>
        auto* elemLLTy = elemTI->elementType->toLLVMType(
            *context_, module_->getDataLayout());
        auto suffix = getVecSuffix(elemTI->elementType);

        auto* idx = std::any_cast<llvm::Value*>(visit(exprs[0]));
        if (idx->getType() != usizeTy)
            idx = builder_->CreateIntCast(idx, usizeTy, false);

        auto* val = std::any_cast<llvm::Value*>(visit(exprs.back()));
        if (val->getType() != elemLLTy) {
            if (val->getType()->isIntegerTy() && elemLLTy->isIntegerTy())
                val = builder_->CreateIntCast(val, elemLLTy,
                    elemTI->elementType->isSigned);
            else if (val->getType()->isFloatingPointTy() &&
                     elemLLTy->isFloatingPointTy()) {
                if (val->getType()->getPrimitiveSizeInBits() >
                    elemLLTy->getPrimitiveSizeInBits())
                    val = builder_->CreateFPTrunc(val, elemLLTy);
                else
                    val = builder_->CreateFPExt(val, elemLLTy);
            }
        }

        auto callee = declareBuiltin(
            "lux_vec_set_" + suffix,
            llvm::Type::getVoidTy(*context_),
            { ptrTy, usizeTy, elemLLTy });
        builder_->CreateCall(callee, { alloca, idx, val });
        return {};
    }

    // ── Array index assignment (original path) ──────────────────────
    auto* val = std::any_cast<llvm::Value*>(visit(exprs.back()));

    auto* i64Ty = llvm::Type::getInt64Ty(*context_);
    std::vector<llvm::Value*> gepIndices;
    gepIndices.push_back(llvm::ConstantInt::get(i64Ty, 0));
    for (size_t i = 0; i + 1 < exprs.size(); i++) {
        auto* idx = std::any_cast<llvm::Value*>(visit(exprs[i]));
        if (idx->getType() != i64Ty)
            idx = builder_->CreateIntCast(idx, i64Ty, true);
        gepIndices.push_back(idx);
    }

    auto* gep = builder_->CreateGEP(allocType, alloca, gepIndices, varName + "_elem");

    // Determine element type by unwrapping array types
    llvm::Type* elemTy = allocType;
    for (size_t i = 0; i + 1 < exprs.size(); i++)
        elemTy = llvm::cast<llvm::ArrayType>(elemTy)->getElementType();

    // Cast value to element type if needed
    if (val->getType() != elemTy) {
        if (val->getType()->isIntegerTy() && elemTy->isIntegerTy())
            val = builder_->CreateIntCast(val, elemTy, elemTI->isSigned);
        else if (val->getType()->isFloatingPointTy() && elemTy->isFloatingPointTy()) {
            if (val->getType()->getPrimitiveSizeInBits() > elemTy->getPrimitiveSizeInBits())
                val = builder_->CreateFPTrunc(val, elemTy);
            else
                val = builder_->CreateFPExt(val, elemTy);
        }
    }

    builder_->CreateStore(val, gep);
    return {};
}

std::any IRGen::visitCompoundAssignStmt(LuxParser::CompoundAssignStmtContext* ctx) {
    auto varName = ctx->IDENTIFIER()->getText();
    auto it = locals_.find(varName);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << varName << "'\n";
        return {};
    }

    auto* alloca = it->second.alloca;
    auto* varTy  = alloca->getAllocatedType();
    auto* elemTI = it->second.typeInfo;

    auto* cur = builder_->CreateLoad(varTy, alloca, varName);
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    // Cast RHS to variable type
    if (rhs->getType() != varTy) {
        if (rhs->getType()->isIntegerTy() && varTy->isIntegerTy())
            rhs = builder_->CreateIntCast(rhs, varTy, elemTI->isSigned);
        else if (rhs->getType()->isFloatingPointTy() && varTy->isFloatingPointTy()) {
            if (rhs->getType()->getPrimitiveSizeInBits() > varTy->getPrimitiveSizeInBits())
                rhs = builder_->CreateFPTrunc(rhs, varTy);
            else
                rhs = builder_->CreateFPExt(rhs, varTy);
        }
    }

    bool isFloat = varTy->isFloatingPointTy();
    llvm::Value* result = nullptr;

    switch (ctx->op->getType()) {
    case LuxLexer::PLUS_ASSIGN:
        result = isFloat ? builder_->CreateFAdd(cur, rhs) : builder_->CreateAdd(cur, rhs);
        break;
    case LuxLexer::MINUS_ASSIGN:
        result = isFloat ? builder_->CreateFSub(cur, rhs) : builder_->CreateSub(cur, rhs);
        break;
    case LuxLexer::STAR_ASSIGN:
        result = isFloat ? builder_->CreateFMul(cur, rhs) : builder_->CreateMul(cur, rhs);
        break;
    case LuxLexer::SLASH_ASSIGN:
        result = isFloat ? builder_->CreateFDiv(cur, rhs) : builder_->CreateSDiv(cur, rhs);
        break;
    case LuxLexer::PERCENT_ASSIGN:
        result = isFloat ? builder_->CreateFRem(cur, rhs) : builder_->CreateSRem(cur, rhs);
        break;
    case LuxLexer::AMP_ASSIGN:
        result = builder_->CreateAnd(cur, rhs);
        break;
    case LuxLexer::PIPE_ASSIGN:
        result = builder_->CreateOr(cur, rhs);
        break;
    case LuxLexer::CARET_ASSIGN:
        result = builder_->CreateXor(cur, rhs);
        break;
    case LuxLexer::LSHIFT_ASSIGN:
        result = builder_->CreateShl(cur, rhs);
        break;
    case LuxLexer::RSHIFT_ASSIGN:
        result = builder_->CreateAShr(cur, rhs);
        break;
    default:
        result = cur;
        break;
    }

    builder_->CreateStore(result, alloca);
    return {};
}

// p.x = 42;
std::any IRGen::visitFieldAssignStmt(LuxParser::FieldAssignStmtContext* ctx) {
    auto identifiers = ctx->IDENTIFIER();
    auto varName = identifiers[0]->getText();

    auto it = locals_.find(varName);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << varName << "'\n";
        return {};
    }

    auto* alloca   = it->second.alloca;
    auto* structTI = it->second.typeInfo;
    auto* structTy = alloca->getAllocatedType();

    // Walk field chain: p.x.y → GEP(0, fieldIdx_x), then GEP(0, fieldIdx_y)
    llvm::Value* currentPtr = alloca;
    llvm::Type*  currentTy  = structTy;
    const TypeInfo* currentTI = structTI;

    for (size_t i = 1; i < identifiers.size(); i++) {
        auto fieldName = identifiers[i]->getText();
        int fieldIdx = -1;
        for (size_t f = 0; f < currentTI->fields.size(); f++) {
            if (currentTI->fields[f].name == fieldName) {
                fieldIdx = static_cast<int>(f);
                break;
            }
        }
        if (fieldIdx < 0) {
            std::cerr << "lux: '" << currentTI->name
                      << "' has no field '" << fieldName << "'\n";
            return {};
        }

        if (currentTI->kind == TypeKind::Union) {
            // Union: all fields at offset 0 — pointer stays the same
            currentTI = currentTI->fields[fieldIdx].typeInfo;
            currentTy = currentTI->toLLVMType(*context_, module_->getDataLayout());
        } else {
            currentPtr = builder_->CreateStructGEP(currentTy, currentPtr,
                                                    fieldIdx, fieldName + "_ptr");
            currentTI = currentTI->fields[fieldIdx].typeInfo;
            currentTy = currentTI->toLLVMType(*context_, module_->getDataLayout());
        }
    }

    auto* val = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    // Cast if needed
    if (val->getType() != currentTy) {
        if (val->getType()->isIntegerTy() && currentTy->isIntegerTy())
            val = builder_->CreateIntCast(val, currentTy, currentTI->isSigned);
        else if (val->getType()->isFloatingPointTy() && currentTy->isFloatingPointTy()) {
            if (val->getType()->getPrimitiveSizeInBits() > currentTy->getPrimitiveSizeInBits())
                val = builder_->CreateFPTrunc(val, currentTy);
            else
                val = builder_->CreateFPExt(val, currentTy);
        }
    }

    builder_->CreateStore(val, currentPtr);
    return {};
}

// arr[i].field = value;  or  arr[i][j].field.subfield = value;
std::any IRGen::visitIndexFieldAssignStmt(
    LuxParser::IndexFieldAssignStmtContext* ctx) {
    auto identifiers = ctx->IDENTIFIER();
    auto varName = identifiers[0]->getText();

    auto it = locals_.find(varName);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << varName << "'\n";
        return {};
    }

    auto* alloca   = it->second.alloca;
    auto* allocType = alloca->getAllocatedType();
    auto* structTI = it->second.typeInfo;
    auto* i64Ty    = llvm::Type::getInt64Ty(*context_);

    // The number of index expressions equals the number of '[' tokens
    size_t numIndices = ctx->LBRACKET().size();
    auto expressions = ctx->expression();
    // expressions layout: [index0, index1, ..., indexN-1, rhs_value]
    // The last expression is always the value to assign

    // Build GEP indices for array: [0, idx0, idx1, ...]
    std::vector<llvm::Value*> gepIndices;
    gepIndices.push_back(llvm::ConstantInt::get(i64Ty, 0));
    for (size_t i = 0; i < numIndices; i++) {
        auto* idx = std::any_cast<llvm::Value*>(visit(expressions[i]));
        if (idx->getType() != i64Ty)
            idx = builder_->CreateIntCast(idx, i64Ty, true);
        gepIndices.push_back(idx);
    }

    // GEP to the array element (a struct)
    auto* elemPtr = builder_->CreateGEP(allocType, alloca, gepIndices,
                                        varName + "_elem_ptr");

    // Determine element type by unwrapping array dimensions
    llvm::Type* elemTy = allocType;
    for (size_t i = 0; i < numIndices; i++)
        elemTy = llvm::cast<llvm::ArrayType>(elemTy)->getElementType();

    // Walk the field chain: identifiers[1], identifiers[2], ...
    llvm::Value* currentPtr = elemPtr;
    llvm::Type*  currentTy  = elemTy;
    const TypeInfo* currentTI = structTI;

    for (size_t i = 1; i < identifiers.size(); i++) {
        auto fieldName = identifiers[i]->getText();
        int fieldIdx = -1;
        for (size_t f = 0; f < currentTI->fields.size(); f++) {
            if (currentTI->fields[f].name == fieldName) {
                fieldIdx = static_cast<int>(f);
                break;
            }
        }
        if (fieldIdx < 0) {
            std::cerr << "lux: '" << currentTI->name
                      << "' has no field '" << fieldName << "'\n";
            return {};
        }

        if (currentTI->kind == TypeKind::Union) {
            currentTI = currentTI->fields[fieldIdx].typeInfo;
            currentTy = currentTI->toLLVMType(*context_, module_->getDataLayout());
        } else {
            currentPtr = builder_->CreateStructGEP(currentTy, currentPtr,
                                                    fieldIdx, fieldName + "_ptr");
            currentTI = currentTI->fields[fieldIdx].typeInfo;
            currentTy = currentTI->toLLVMType(*context_, module_->getDataLayout());
        }
    }

    // The RHS value is the last expression
    auto* val = std::any_cast<llvm::Value*>(visit(expressions[expressions.size() - 1]));

    // Cast if needed
    if (val->getType() != currentTy) {
        if (val->getType()->isIntegerTy() && currentTy->isIntegerTy())
            val = builder_->CreateIntCast(val, currentTy, currentTI->isSigned);
        else if (val->getType()->isFloatingPointTy() && currentTy->isFloatingPointTy()) {
            if (val->getType()->getPrimitiveSizeInBits() > currentTy->getPrimitiveSizeInBits())
                val = builder_->CreateFPTrunc(val, currentTy);
            else
                val = builder_->CreateFPExt(val, currentTy);
        }
    }

    builder_->CreateStore(val, currentPtr);
    return {};
}

// ptr->field = value;
std::any IRGen::visitArrowAssignStmt(LuxParser::ArrowAssignStmtContext* ctx) {
    auto identifiers = ctx->IDENTIFIER();
    auto varName = identifiers[0]->getText();
    auto fieldName = identifiers[1]->getText();

    auto it = locals_.find(varName);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << varName << "'\n";
        return {};
    }

    // Load the pointer from the alloca
    auto* ptrTy = llvm::PointerType::getUnqual(*context_);
    auto* ptrVal = builder_->CreateLoad(ptrTy, it->second.alloca, varName + "_ptr");

    // Resolve struct type info
    const TypeInfo* structTI = it->second.typeInfo;
    if (structTI->kind == TypeKind::Pointer)
        structTI = structTI->pointeeType;

    if (!structTI || (structTI->kind != TypeKind::Struct && structTI->kind != TypeKind::Union)) {
        std::cerr << "lux: '->' requires pointer to struct or union\n";
        return {};
    }

    int fieldIdx = -1;
    const TypeInfo* fieldTI = nullptr;
    for (size_t f = 0; f < structTI->fields.size(); f++) {
        if (structTI->fields[f].name == fieldName) {
            fieldIdx = static_cast<int>(f);
            fieldTI = structTI->fields[f].typeInfo;
            break;
        }
    }
    if (fieldIdx < 0) {
        std::cerr << "lux: '" << structTI->name
                  << "' has no field '" << fieldName << "'\n";
        return {};
    }

    auto* val = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    auto* fieldLLTy = fieldTI->toLLVMType(*context_, module_->getDataLayout());

    if (val->getType() != fieldLLTy) {
        if (val->getType()->isIntegerTy() && fieldLLTy->isIntegerTy())
            val = builder_->CreateIntCast(val, fieldLLTy, fieldTI->isSigned);
        else if (val->getType()->isFloatingPointTy() && fieldLLTy->isFloatingPointTy())
            val = builder_->CreateFPCast(val, fieldLLTy);
    }

    if (structTI->kind == TypeKind::Union) {
        // Union: store at offset 0 (same as ptrVal)
        builder_->CreateStore(val, ptrVal);
    } else {
        auto* structLLTy = structTI->toLLVMType(*context_, module_->getDataLayout());
        auto* gep = builder_->CreateStructGEP(structLLTy, ptrVal, fieldIdx, fieldName + "_ptr");
        builder_->CreateStore(val, gep);
    }
    return {};
}

// ptr->field += value;
std::any IRGen::visitArrowCompoundAssignStmt(
    LuxParser::ArrowCompoundAssignStmtContext* ctx) {
    auto identifiers = ctx->IDENTIFIER();
    auto varName = identifiers[0]->getText();
    auto fieldName = identifiers[1]->getText();

    auto it = locals_.find(varName);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << varName << "'\n";
        return {};
    }

    auto* ptrTy = llvm::PointerType::getUnqual(*context_);
    auto* ptrVal = builder_->CreateLoad(ptrTy, it->second.alloca, varName + "_ptr");

    const TypeInfo* structTI = it->second.typeInfo;
    if (structTI->kind == TypeKind::Pointer)
        structTI = structTI->pointeeType;

    if (!structTI || structTI->kind != TypeKind::Struct) {
        std::cerr << "lux: '->' requires pointer to struct\n";
        return {};
    }

    int fieldIdx = -1;
    for (size_t f = 0; f < structTI->fields.size(); f++) {
        if (structTI->fields[f].name == fieldName) {
            fieldIdx = static_cast<int>(f);
            break;
        }
    }
    if (fieldIdx < 0) {
        std::cerr << "lux: struct '" << structTI->name
                  << "' has no field '" << fieldName << "'\n";
        return {};
    }

    auto* structLLTy = structTI->toLLVMType(*context_, module_->getDataLayout());
    auto* gep = builder_->CreateStructGEP(structLLTy, ptrVal, fieldIdx, fieldName + "_ptr");
    auto* fieldTy = structTI->fields[fieldIdx].typeInfo
                        ->toLLVMType(*context_, module_->getDataLayout());

    auto* oldVal = builder_->CreateLoad(fieldTy, gep, fieldName + "_old");
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    if (rhs->getType() != fieldTy) {
        if (rhs->getType()->isIntegerTy() && fieldTy->isIntegerTy())
            rhs = builder_->CreateIntCast(rhs, fieldTy,
                      structTI->fields[fieldIdx].typeInfo->isSigned);
    }

    llvm::Value* result;
    auto opType = ctx->op->getType();
    bool isFloat = fieldTy->isFloatingPointTy();

    if (opType == LuxLexer::PLUS_ASSIGN)
        result = isFloat ? builder_->CreateFAdd(oldVal, rhs) : builder_->CreateAdd(oldVal, rhs);
    else if (opType == LuxLexer::MINUS_ASSIGN)
        result = isFloat ? builder_->CreateFSub(oldVal, rhs) : builder_->CreateSub(oldVal, rhs);
    else if (opType == LuxLexer::STAR_ASSIGN)
        result = isFloat ? builder_->CreateFMul(oldVal, rhs) : builder_->CreateMul(oldVal, rhs);
    else if (opType == LuxLexer::SLASH_ASSIGN)
        result = isFloat ? builder_->CreateFDiv(oldVal, rhs)
                         : (structTI->fields[fieldIdx].typeInfo->isSigned
                                ? builder_->CreateSDiv(oldVal, rhs)
                                : builder_->CreateUDiv(oldVal, rhs));
    else if (opType == LuxLexer::PERCENT_ASSIGN)
        result = isFloat ? builder_->CreateFRem(oldVal, rhs)
                         : (structTI->fields[fieldIdx].typeInfo->isSigned
                                ? builder_->CreateSRem(oldVal, rhs)
                                : builder_->CreateURem(oldVal, rhs));
    else if (opType == LuxLexer::AMP_ASSIGN)
        result = builder_->CreateAnd(oldVal, rhs);
    else if (opType == LuxLexer::PIPE_ASSIGN)
        result = builder_->CreateOr(oldVal, rhs);
    else if (opType == LuxLexer::CARET_ASSIGN)
        result = builder_->CreateXor(oldVal, rhs);
    else if (opType == LuxLexer::LSHIFT_ASSIGN)
        result = builder_->CreateShl(oldVal, rhs);
    else if (opType == LuxLexer::RSHIFT_ASSIGN)
        result = structTI->fields[fieldIdx].typeInfo->isSigned
                     ? builder_->CreateAShr(oldVal, rhs)
                     : builder_->CreateLShr(oldVal, rhs);
    else {
        std::cerr << "lux: unsupported compound assign operator\n";
        return {};
    }

    builder_->CreateStore(result, gep);
    return {};
}

std::any IRGen::visitExprStmt(LuxParser::ExprStmtContext* ctx) {
    visit(ctx->expression());
    return {};
}

// println(x);
std::any IRGen::visitCallStmt(LuxParser::CallStmtContext* ctx) {
    auto funcName = ctx->IDENTIFIER()->getText();

    // ── Global builtins (always available, no import required) ──────────
    if (globalBuiltins_.count(funcName)) {
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* voidTy  = llvm::Type::getVoidTy(*context_);
        auto* i32Ty   = llvm::Type::getInt32Ty(*context_);
        auto* i1Ty    = llvm::Type::getInt1Ty(*context_);

        // Collect argument values
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }

        if (funcName == "exit") {
            auto* arg = args.empty()
                ? llvm::ConstantInt::get(i32Ty, 0)
                : builder_->CreateIntCast(args[0], i32Ty, true, "exit_code");
            auto callee = declareBuiltin("lux_exit", voidTy, {i32Ty});
            builder_->CreateCall(callee, {arg});
        } else if (funcName == "panic") {
            auto* strPtr = builder_->CreateExtractValue(args[0], 0, "panic_ptr");
            auto* strLen = builder_->CreateExtractValue(args[0], 1, "panic_len");
            auto callee = declareBuiltin("lux_panic", voidTy, {ptrTy, usizeTy});
            builder_->CreateCall(callee, {strPtr, strLen});
        } else if (funcName == "assert") {
            auto* cond = args[0];
            if (!cond->getType()->isIntegerTy(32))
                cond = builder_->CreateZExt(cond, i32Ty, "assert_cond");
            // Inject file name and line number at compile time
            auto fileName = module_->getName().str();
            auto* fileGlobal = builder_->CreateGlobalString(fileName, ".assert_file", 0, module_);
            auto* fileLen = llvm::ConstantInt::get(usizeTy, fileName.size());
            auto* lineNo  = llvm::ConstantInt::get(i32Ty, ctx->getStart()->getLine());
            auto callee = declareBuiltin("lux_assert", voidTy,
                                         {i32Ty, ptrTy, usizeTy, i32Ty});
            builder_->CreateCall(callee, {cond, fileGlobal, fileLen, lineNo});
        } else if (funcName == "assertMsg") {
            auto* cond = args[0];
            if (!cond->getType()->isIntegerTy(32))
                cond = builder_->CreateZExt(cond, i32Ty, "assertmsg_cond");
            auto* msgPtr = builder_->CreateExtractValue(args[1], 0, "assertmsg_ptr");
            auto* msgLen = builder_->CreateExtractValue(args[1], 1, "assertmsg_len");
            auto callee = declareBuiltin("lux_assertMsg", voidTy,
                                         {i32Ty, ptrTy, usizeTy});
            builder_->CreateCall(callee, {cond, msgPtr, msgLen});
        } else if (funcName == "unreachable") {
            auto fileName = module_->getName().str();
            auto* fileGlobal = builder_->CreateGlobalString(fileName, ".unreach_file", 0, module_);
            auto* fileLen = llvm::ConstantInt::get(usizeTy, fileName.size());
            auto* lineNo  = llvm::ConstantInt::get(i32Ty, ctx->getStart()->getLine());
            auto callee = declareBuiltin("lux_unreachable", voidTy,
                                         {ptrTy, usizeTy, i32Ty});
            builder_->CreateCall(callee, {fileGlobal, fileLen, lineNo});
        }
        return {};
    }

    // ── User-defined or extern C function called as statement ──────
    auto resolvedName = resolveCallTarget(funcName);
    auto* userFn = module_->getFunction(resolvedName);

    // Lazy declare C function from #include bindings
    if (!userFn)
        userFn = declareCFunction(funcName);

    if (userFn) {
        std::vector<llvm::Value*> rawArgs;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                rawArgs.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }

        // ── C extern variadic function (e.g. printf) ───────────────
        if (externCFunctions_.count(resolvedName) && userFn->isVarArg()) {
            auto* fnType = userFn->getFunctionType();
            unsigned fixedParams = fnType->getNumParams();

            for (size_t i = 0; i < rawArgs.size(); i++) {
                auto* a = rawArgs[i];
                if (i < fixedParams) {
                    // Fixed params: coerce to declared type
                    if (a->getType() != fnType->getParamType(i)) {
                        if (a->getType()->isIntegerTy() && fnType->getParamType(i)->isIntegerTy())
                            a = builder_->CreateIntCast(a, fnType->getParamType(i), true);
                        else if (a->getType()->isFloatingPointTy() && fnType->getParamType(i)->isFloatingPointTy())
                            a = builder_->CreateFPExt(a, fnType->getParamType(i));
                    }
                } else {
                    // Variadic args: apply C default argument promotions
                    if (a->getType()->isFloatTy())
                        a = builder_->CreateFPExt(a, llvm::Type::getDoubleTy(*context_), "c_promote_f64");
                    else if (a->getType()->isIntegerTy()) {
                        unsigned bw = a->getType()->getIntegerBitWidth();
                        if (bw < 32) {
                            if (bw == 1)
                                a = builder_->CreateZExt(a, llvm::Type::getInt32Ty(*context_), "c_promote_bool");
                            else
                                a = builder_->CreateSExt(a, llvm::Type::getInt32Ty(*context_), "c_promote_i32");
                        }
                        else if (bw > 64)
                            a = builder_->CreateTrunc(a, llvm::Type::getInt32Ty(*context_), "c_trunc_i32");
                    }
                }
                rawArgs[i] = a;
            }
            builder_->CreateCall(userFn, rawArgs);
            return {};
        }

        auto vit = variadicFunctions_.find(resolvedName);
        if (vit != variadicFunctions_.end()) {
            auto& vfInfo = vit->second;
            size_t varIdx = vfInfo.variadicParamIdx;
            auto* elemTy = vfInfo.elementType->toLLVMType(*context_, module_->getDataLayout());
            auto* fnType = userFn->getFunctionType();

            std::vector<llvm::Value*> callArgs;
            std::vector<llvm::Value*> variadicArgs;

            for (size_t i = 0; i < rawArgs.size(); i++) {
                auto* a = rawArgs[i];
                if (i < varIdx) {
                    if (a->getType() != fnType->getParamType(i)) {
                        if (a->getType()->isIntegerTy() && fnType->getParamType(i)->isIntegerTy())
                            a = builder_->CreateIntCast(a, fnType->getParamType(i), true);
                        else if (a->getType()->isFloatingPointTy() && fnType->getParamType(i)->isFloatingPointTy()) {
                            if (a->getType()->getPrimitiveSizeInBits() > fnType->getParamType(i)->getPrimitiveSizeInBits())
                                a = builder_->CreateFPTrunc(a, fnType->getParamType(i));
                            else
                                a = builder_->CreateFPExt(a, fnType->getParamType(i));
                        }
                    }
                    callArgs.push_back(a);
                } else {
                    if (a->getType() != elemTy) {
                        if (a->getType()->isIntegerTy() && elemTy->isIntegerTy())
                            a = builder_->CreateIntCast(a, elemTy, vfInfo.elementType->isSigned);
                        else if (a->getType()->isFloatingPointTy() && elemTy->isFloatingPointTy()) {
                            if (a->getType()->getPrimitiveSizeInBits() > elemTy->getPrimitiveSizeInBits())
                                a = builder_->CreateFPTrunc(a, elemTy);
                            else
                                a = builder_->CreateFPExt(a, elemTy);
                        }
                    }
                    variadicArgs.push_back(a);
                }
            }

            auto* i64Ty = llvm::Type::getInt64Ty(*context_);
            auto* ptrTy = llvm::PointerType::getUnqual(*context_);

            if (variadicArgs.empty()) {
                callArgs.push_back(llvm::ConstantPointerNull::get(ptrTy));
                callArgs.push_back(llvm::ConstantInt::get(i64Ty, 0));
            } else {
                auto* arrTy = llvm::ArrayType::get(elemTy, variadicArgs.size());
                auto* arrAlloca = builder_->CreateAlloca(arrTy, nullptr, "variadic.pack");
                auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                for (size_t j = 0; j < variadicArgs.size(); j++) {
                    auto* idx = llvm::ConstantInt::get(i64Ty, j);
                    auto* gep = builder_->CreateGEP(arrTy, arrAlloca, {zero, idx});
                    builder_->CreateStore(variadicArgs[j], gep);
                }
                callArgs.push_back(arrAlloca);
                callArgs.push_back(llvm::ConstantInt::get(i64Ty, variadicArgs.size()));
            }

            builder_->CreateCall(userFn, callArgs);
        } else {
            auto* fnType = userFn->getFunctionType();

            // Check if this C function needs ABI struct coercion
            auto abiIt = cabiInfos_.find(userFn->getName().str());
            bool needsABI = (abiIt != cabiInfos_.end());
            CABIInfo* abi = needsABI ? &abiIt->second : nullptr;

            std::vector<llvm::Value*> args;

            // Sret: allocate space for large struct return, pass as hidden first arg
            llvm::AllocaInst* sretAlloca = nullptr;
            if (abi && abi->returnInfo.kind == ABIArgKind::Indirect) {
                sretAlloca = builder_->CreateAlloca(abi->originalRetType, nullptr, "sret");
                args.push_back(sretAlloca);
            }

            for (size_t i = 0; i < rawArgs.size(); i++) {
                auto* argVal = rawArgs[i];

                // ABI struct coercion for C functions
                if (abi && i < abi->paramInfos.size()) {
                    auto& pi = abi->paramInfos[i];
                    if (pi.kind == ABIArgKind::Coerce && argVal->getType()->isStructTy()) {
                        auto* tmp = builder_->CreateAlloca(argVal->getType(), nullptr, "abi.coerce");
                        builder_->CreateStore(argVal, tmp);
                        argVal = builder_->CreateLoad(pi.coercedType, tmp, "abi.coerced");
                        args.push_back(argVal);
                        continue;
                    }
                    if (pi.kind == ABIArgKind::Expand && argVal->getType()->isStructTy()) {
                        auto* tmp = builder_->CreateAlloca(argVal->getType(), nullptr, "abi.expand");
                        builder_->CreateStore(argVal, tmp);
                        auto* i8Ty = llvm::Type::getInt8Ty(*context_);
                        uint64_t offset = 0;
                        for (auto* partTy : pi.expandedTypes) {
                            llvm::Value* partPtr = tmp;
                            if (offset > 0)
                                partPtr = builder_->CreateGEP(i8Ty, tmp,
                                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), offset));
                            args.push_back(builder_->CreateLoad(partTy, partPtr, "abi.part"));
                            offset += module_->getDataLayout().getTypeAllocSize(partTy);
                        }
                        continue;
                    }
                    if (pi.kind == ABIArgKind::Indirect && argVal->getType()->isStructTy()) {
                        auto* tmp = builder_->CreateAlloca(argVal->getType(), nullptr, "abi.byval");
                        builder_->CreateStore(argVal, tmp);
                        args.push_back(tmp);
                        continue;
                    }
                }

                // Standard type coercion
                unsigned llvmIdx = static_cast<unsigned>(i);
                if (sretAlloca) llvmIdx++;
                if (llvmIdx < fnType->getNumParams() && argVal->getType() != fnType->getParamType(llvmIdx)) {
                    auto* expectedTy = fnType->getParamType(llvmIdx);
                    if (argVal->getType()->isArrayTy() && expectedTy->isPointerTy()) {
                        auto* tmpAlloca = builder_->CreateAlloca(argVal->getType(), nullptr, "arr.decay");
                        builder_->CreateStore(argVal, tmpAlloca);
                        auto* zero = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0);
                        argVal = builder_->CreateGEP(argVal->getType(), tmpAlloca, {zero, zero}, "arr.ptr");
                    } else if (argVal->getType()->isIntegerTy() && expectedTy->isIntegerTy())
                        argVal = builder_->CreateIntCast(argVal, expectedTy, true);
                    else if (argVal->getType()->isFloatingPointTy() && expectedTy->isFloatingPointTy()) {
                        if (argVal->getType()->getPrimitiveSizeInBits() > expectedTy->getPrimitiveSizeInBits())
                            argVal = builder_->CreateFPTrunc(argVal, expectedTy);
                        else
                            argVal = builder_->CreateFPExt(argVal, expectedTy);
                    }
                }
                args.push_back(argVal);
            }
            builder_->CreateCall(userFn, args);
        }
        return {};
    }

    if (!imports_.isImported(funcName)) {
        std::cerr << "lux: call to undeclared function '" << funcName
                  << "'. Did you forget 'use std::log::" << funcName << ";'?\n";
        return {};
    }

    // ── dbg() as statement: print debug info to stderr ──────────────────
    if (funcName == "dbg") {
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* i32Ty   = llvm::Type::getInt32Ty(*context_);

        // Inject file name and line number
        auto fileName = module_->getName().str();
        auto* fileGlobal = builder_->CreateGlobalString(fileName, ".dbg_file", 0, module_);
        auto* fileLen = llvm::ConstantInt::get(usizeTy, fileName.size());
        auto* lineNo  = llvm::ConstantInt::get(i32Ty, ctx->getStart()->getLine());

        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }

        for (auto* arg : args) {
            auto* argType = arg->getType();
            const TypeInfo* argTypeInfo = nullptr;
            if (auto* load = llvm::dyn_cast<llvm::LoadInst>(arg)) {
                auto* src = load->getPointerOperand();
                for (auto& [vname, info] : locals_) {
                    if (info.alloca == src) { argTypeInfo = info.typeInfo; break; }
                }
            }

            bool isString = (argTypeInfo && argTypeInfo->kind == TypeKind::String)
                         || (!argTypeInfo && argType->isStructTy());

            if (isString) {
                auto* strPtr = builder_->CreateExtractValue(arg, 0, "dbg_str_ptr");
                auto* strLen = builder_->CreateExtractValue(arg, 1, "dbg_str_len");
                auto callee = declareBuiltin("lux_dbg_str",
                    llvm::Type::getVoidTy(*context_),
                    {ptrTy, usizeTy, i32Ty, ptrTy, usizeTy});
                builder_->CreateCall(callee, {fileGlobal, fileLen, lineNo, strPtr, strLen});
                continue;
            }

            std::string suffix;
            if (argTypeInfo)                      suffix = argTypeInfo->builtinSuffix;
            else if (argType->isIntegerTy(1))     suffix = "bool";
            else if (argType->isIntegerTy(8))     suffix = "i8";
            else if (argType->isIntegerTy(16))    suffix = "i16";
            else if (argType->isIntegerTy(32))    suffix = "i32";
            else if (argType->isIntegerTy(64))    suffix = "i64";
            else if (argType->isIntegerTy(128))   suffix = "i128";
            else if (argType->isFloatTy())        suffix = "f32";
            else if (argType->isDoubleTy())       suffix = "f64";
            else                                  suffix = "i32";

            llvm::Value* callArg = arg;
            llvm::Type*  paramType = argType;
            if (argType->isIntegerTy(256)) {
                paramType = llvm::Type::getInt128Ty(*context_);
                callArg   = builder_->CreateTrunc(arg, paramType);
            }

            auto cFuncName = "lux_dbg_" + suffix;
            auto callee = declareBuiltin(cFuncName,
                llvm::Type::getVoidTy(*context_),
                {ptrTy, usizeTy, i32Ty, paramType});
            builder_->CreateCall(callee, {fileGlobal, fileLen, lineNo, callArg});
        }
        return {};
    }

    // ── std::io void functions as statement ─────────────────────────────
    if (funcName == "flush") {
        auto callee = declareBuiltin("lux_flush",
            llvm::Type::getVoidTy(*context_), {});
        builder_->CreateCall(callee, {});
        return {};
    }
    if (funcName == "flushErr") {
        auto callee = declareBuiltin("lux_flushErr",
            llvm::Type::getVoidTy(*context_), {});
        builder_->CreateCall(callee, {});
        return {};
    }

    // ── std::mem void functions as statement ─────────────────────────
    if (funcName == "free") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy  = llvm::PointerType::getUnqual(*context_);
        auto callee = declareBuiltin("lux_free",
            llvm::Type::getVoidTy(*context_), {ptrTy});
        builder_->CreateCall(callee, {args[0]});
        return {};
    }
    if (funcName == "copy") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* n = args[2];
        if (n->getType() != usizeTy)
            n = builder_->CreateIntCast(n, usizeTy, false);
        auto callee = declareBuiltin("lux_copy",
            llvm::Type::getVoidTy(*context_), {ptrTy, ptrTy, usizeTy});
        builder_->CreateCall(callee, {args[0], args[1], n});
        return {};
    }
    if (funcName == "move") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* n = args[2];
        if (n->getType() != usizeTy)
            n = builder_->CreateIntCast(n, usizeTy, false);
        auto callee = declareBuiltin("lux_move",
            llvm::Type::getVoidTy(*context_), {ptrTy, ptrTy, usizeTy});
        builder_->CreateCall(callee, {args[0], args[1], n});
        return {};
    }
    if (funcName == "set") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* i8Ty    = llvm::Type::getInt8Ty(*context_);
        auto* val = args[1];
        if (val->getType() != i8Ty)
            val = builder_->CreateIntCast(val, i8Ty, false);
        auto* n = args[2];
        if (n->getType() != usizeTy)
            n = builder_->CreateIntCast(n, usizeTy, false);
        auto callee = declareBuiltin("lux_set",
            llvm::Type::getVoidTy(*context_), {ptrTy, i8Ty, usizeTy});
        builder_->CreateCall(callee, {args[0], val, n});
        return {};
    }
    if (funcName == "zero") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* n = args[1];
        if (n->getType() != usizeTy)
            n = builder_->CreateIntCast(n, usizeTy, false);
        auto callee = declareBuiltin("lux_zero",
            llvm::Type::getVoidTy(*context_), {ptrTy, usizeTy});
        builder_->CreateCall(callee, {args[0], n});
        return {};
    }

    // ── std::random void functions as statement ─────────────────────
    if (funcName == "seed") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* u64Ty = llvm::Type::getInt64Ty(*context_);
        auto* s = args[0];
        if (s->getType() != u64Ty)
            s = builder_->CreateIntCast(s, u64Ty, false);
        auto callee = declareBuiltin("lux_seed",
            llvm::Type::getVoidTy(*context_), {u64Ty});
        builder_->CreateCall(callee, {s});
        return {};
    }
    if (funcName == "seedTime") {
        auto callee = declareBuiltin("lux_seedTime",
            llvm::Type::getVoidTy(*context_), {});
        builder_->CreateCall(callee, {});
        return {};
    }

    // ── std::time void functions as statement ───────────────────────
    if (funcName == "sleep") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* u64Ty = llvm::Type::getInt64Ty(*context_);
        auto* ms = args[0];
        if (ms->getType() != u64Ty)
            ms = builder_->CreateIntCast(ms, u64Ty, false);
        auto callee = declareBuiltin("lux_sleep",
            llvm::Type::getVoidTy(*context_), {u64Ty});
        builder_->CreateCall(callee, {ms});
        return {};
    }
    if (funcName == "sleepMicros") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* u64Ty = llvm::Type::getInt64Ty(*context_);
        auto* us = args[0];
        if (us->getType() != u64Ty)
            us = builder_->CreateIntCast(us, u64Ty, false);
        auto callee = declareBuiltin("lux_sleepMicros",
            llvm::Type::getVoidTy(*context_), {u64Ty});
        builder_->CreateCall(callee, {us});
        return {};
    }

    // ── std::fs void functions as statement ─────────────────────────
    if (funcName == "writeFile" || funcName == "appendFile") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* voidTy  = llvm::Type::getVoidTy(*context_);

        auto* pathPtr = builder_->CreateExtractValue(args[0], 0, "fs_path_ptr");
        auto* pathLen = builder_->CreateExtractValue(args[0], 1, "fs_path_len");
        auto* dataPtr = builder_->CreateExtractValue(args[1], 0, "fs_data_ptr");
        auto* dataLen = builder_->CreateExtractValue(args[1], 1, "fs_data_len");

        auto callee2 = declareBuiltin("lux_" + funcName, voidTy,
            {ptrTy, usizeTy, ptrTy, usizeTy});
        builder_->CreateCall(callee2, {pathPtr, pathLen, dataPtr, dataLen});
        return {};
    }

    // ── std::fs bool-returning functions called as statement (discard result) ─
    {
        static const std::unordered_set<std::string> fsBoolStmt1 = {
            "exists", "isFile", "isDir",
            "remove", "removeDir",
            "mkdir", "mkdirAll", "setCwd"
        };
        if (fsBoolStmt1.count(funcName)) {
            std::vector<llvm::Value*> args;
            if (auto* argList = ctx->argList()) {
                for (auto* exprCtx : argList->expression())
                    args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
            }
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
            auto* i32Ty   = llvm::Type::getInt32Ty(*context_);

            auto* pathPtr = builder_->CreateExtractValue(args[0], 0, "fs_path_ptr");
            auto* pathLen = builder_->CreateExtractValue(args[0], 1, "fs_path_len");
            auto callee2 = declareBuiltin("lux_" + funcName, i32Ty,
                {ptrTy, usizeTy});
            builder_->CreateCall(callee2, {pathPtr, pathLen});
            return {};
        }
    }
    if (funcName == "rename") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* i32Ty   = llvm::Type::getInt32Ty(*context_);

        auto* fromPtr = builder_->CreateExtractValue(args[0], 0, "fs_from_ptr");
        auto* fromLen = builder_->CreateExtractValue(args[0], 1, "fs_from_len");
        auto* toPtr   = builder_->CreateExtractValue(args[1], 0, "fs_to_ptr");
        auto* toLen   = builder_->CreateExtractValue(args[1], 1, "fs_to_len");
        auto callee2 = declareBuiltin("lux_fsRename", i32Ty,
            {ptrTy, usizeTy, ptrTy, usizeTy});
        builder_->CreateCall(callee2, {fromPtr, fromLen, toPtr, toLen});
        return {};
    }

    // ── std::process void functions as statement ────────────────────
    if (funcName == "abort") {
        auto callee2 = declareBuiltin("lux_abort",
            llvm::Type::getVoidTy(*context_), {});
        builder_->CreateCall(callee2, {});
        builder_->CreateUnreachable();
        return {};
    }
    if (funcName == "setEnv") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* voidTy  = llvm::Type::getVoidTy(*context_);

        auto* namePtr = builder_->CreateExtractValue(args[0], 0, "env_name_ptr");
        auto* nameLen = builder_->CreateExtractValue(args[0], 1, "env_name_len");
        auto* valPtr  = builder_->CreateExtractValue(args[1], 0, "env_val_ptr");
        auto* valLen  = builder_->CreateExtractValue(args[1], 1, "env_val_len");
        auto callee2 = declareBuiltin("lux_setEnv", voidTy,
            {ptrTy, usizeTy, ptrTy, usizeTy});
        builder_->CreateCall(callee2, {namePtr, nameLen, valPtr, valLen});
        return {};
    }
    if (funcName == "exec") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* exprCtx : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
        }
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* i32Ty   = llvm::Type::getInt32Ty(*context_);

        auto* cmdPtr = builder_->CreateExtractValue(args[0], 0, "cmd_ptr");
        auto* cmdLen = builder_->CreateExtractValue(args[0], 1, "cmd_len");
        auto callee2 = declareBuiltin("lux_exec", i32Ty,
            {ptrTy, usizeTy});
        builder_->CreateCall(callee2, {cmdPtr, cmdLen});
        return {};
    }

    // ── std::test — assertEqual(T, T) as statement ─────────────────
    if (funcName == "assertEqual" || funcName == "assertNotEqual") {
        std::vector<llvm::Value*> tArgs;
        if (auto* argList = ctx->argList())
            for (auto* e : argList->expression())
                tArgs.push_back(std::any_cast<llvm::Value*>(visit(e)));
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* voidTy  = llvm::Type::getVoidTy(*context_);
        auto* i8Ty    = llvm::Type::getInt8Ty(*context_);
        auto* i32Ty   = llvm::Type::getInt32Ty(*context_);
        auto* i64Ty   = llvm::Type::getInt64Ty(*context_);
        auto* f64Ty   = llvm::Type::getDoubleTy(*context_);
        auto* strTy   = llvm::StructType::get(*context_, {ptrTy, usizeTy});
        auto* i1Ty    = llvm::Type::getInt1Ty(*context_);
        std::string prefix = (funcName == "assertEqual")
            ? "lux_assertEqual" : "lux_assertNotEqual";
        auto* ty = tArgs[0]->getType();
        if (ty == strTy) {
            std::vector<llvm::Value*> callArgs;
            callArgs.push_back(builder_->CreateExtractValue(tArgs[0], 0));
            callArgs.push_back(builder_->CreateExtractValue(tArgs[0], 1));
            callArgs.push_back(builder_->CreateExtractValue(tArgs[1], 0));
            callArgs.push_back(builder_->CreateExtractValue(tArgs[1], 1));
            auto callee = declareBuiltin(prefix + "Str", voidTy,
                {ptrTy, usizeTy, ptrTy, usizeTy});
            builder_->CreateCall(callee, callArgs);
        } else if (ty == i1Ty) {
            auto* a = builder_->CreateZExt(tArgs[0], i32Ty);
            auto* b = builder_->CreateZExt(tArgs[1], i32Ty);
            auto callee = declareBuiltin(prefix + "Bool", voidTy, {i32Ty, i32Ty});
            builder_->CreateCall(callee, {a, b});
        } else if (ty == i8Ty) {
            auto callee = declareBuiltin(prefix + "Char", voidTy, {i8Ty, i8Ty});
            builder_->CreateCall(callee, {tArgs[0], tArgs[1]});
        } else if (ty->isDoubleTy()) {
            auto callee = declareBuiltin(prefix + "F64", voidTy, {f64Ty, f64Ty});
            builder_->CreateCall(callee, {tArgs[0], tArgs[1]});
        } else {
            auto* a = builder_->CreateIntCast(tArgs[0], i64Ty, true);
            auto* b = builder_->CreateIntCast(tArgs[1], i64Ty, true);
            auto callee = declareBuiltin(prefix + "I64", voidTy, {i64Ty, i64Ty});
            builder_->CreateCall(callee, {a, b});
        }
        return {};
    }

    // ── std::test — assertTrue / assertFalse as statement ──────────
    if (funcName == "assertTrue" || funcName == "assertFalse") {
        std::vector<llvm::Value*> tArgs;
        if (auto* argList = ctx->argList())
            for (auto* e : argList->expression())
                tArgs.push_back(std::any_cast<llvm::Value*>(visit(e)));
        auto* voidTy = llvm::Type::getVoidTy(*context_);
        auto* i32Ty  = llvm::Type::getInt32Ty(*context_);
        auto* cond = builder_->CreateZExt(tArgs[0], i32Ty);
        std::string cName = (funcName == "assertTrue")
            ? "lux_assertTrue" : "lux_assertFalse";
        auto callee = declareBuiltin(cName, voidTy, {i32Ty});
        builder_->CreateCall(callee, {cond});
        return {};
    }

    // ── std::test — assertGreater/Less/GreaterEq/LessEq as statement ─
    {
        static const std::unordered_map<std::string, std::string> cmpI64 = {
            {"assertGreater",   "lux_assertGreaterI64"},
            {"assertLess",      "lux_assertLessI64"},
            {"assertGreaterEq", "lux_assertGreaterEqI64"},
            {"assertLessEq",    "lux_assertLessEqI64"},
        };
        static const std::unordered_map<std::string, std::string> cmpF64 = {
            {"assertGreater",   "lux_assertGreaterF64"},
            {"assertLess",      "lux_assertLessF64"},
            {"assertGreaterEq", "lux_assertGreaterEqF64"},
            {"assertLessEq",    "lux_assertLessEqF64"},
        };
        auto itI = cmpI64.find(funcName);
        if (itI != cmpI64.end()) {
            std::vector<llvm::Value*> tArgs;
            if (auto* argList = ctx->argList())
                for (auto* e : argList->expression())
                    tArgs.push_back(std::any_cast<llvm::Value*>(visit(e)));
            auto* voidTy = llvm::Type::getVoidTy(*context_);
            auto* i64Ty  = llvm::Type::getInt64Ty(*context_);
            auto* f64Ty  = llvm::Type::getDoubleTy(*context_);
            auto* ty = tArgs[0]->getType();
            if (ty->isDoubleTy()) {
                auto callee = declareBuiltin(cmpF64.at(funcName), voidTy,
                    {f64Ty, f64Ty});
                builder_->CreateCall(callee, {tArgs[0], tArgs[1]});
            } else {
                auto* a = builder_->CreateIntCast(tArgs[0], i64Ty, true);
                auto* b = builder_->CreateIntCast(tArgs[1], i64Ty, true);
                auto callee = declareBuiltin(itI->second, voidTy,
                    {i64Ty, i64Ty});
                builder_->CreateCall(callee, {a, b});
            }
            return {};
        }
    }

    // ── std::test — assertStringContains(string, string) as statement ─
    if (funcName == "assertStringContains") {
        std::vector<llvm::Value*> tArgs;
        if (auto* argList = ctx->argList())
            for (auto* e : argList->expression())
                tArgs.push_back(std::any_cast<llvm::Value*>(visit(e)));
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* voidTy  = llvm::Type::getVoidTy(*context_);
        std::vector<llvm::Value*> callArgs;
        callArgs.push_back(builder_->CreateExtractValue(tArgs[0], 0));
        callArgs.push_back(builder_->CreateExtractValue(tArgs[0], 1));
        callArgs.push_back(builder_->CreateExtractValue(tArgs[1], 0));
        callArgs.push_back(builder_->CreateExtractValue(tArgs[1], 1));
        auto callee = declareBuiltin("lux_assertStringContains", voidTy,
            {ptrTy, usizeTy, ptrTy, usizeTy});
        builder_->CreateCall(callee, callArgs);
        return {};
    }

    // ── std::test — assertNear(f64, f64, f64) as statement ────────
    if (funcName == "assertNear") {
        std::vector<llvm::Value*> tArgs;
        if (auto* argList = ctx->argList())
            for (auto* e : argList->expression())
                tArgs.push_back(std::any_cast<llvm::Value*>(visit(e)));
        auto* voidTy = llvm::Type::getVoidTy(*context_);
        auto* f64Ty  = llvm::Type::getDoubleTy(*context_);
        auto callee = declareBuiltin("lux_assertNear", voidTy,
            {f64Ty, f64Ty, f64Ty});
        builder_->CreateCall(callee, {tArgs[0], tArgs[1], tArgs[2]});
        return {};
    }

    // ── std::test — fail / skip / log (string) as statement ───────
    {
        static const std::unordered_map<std::string, std::string> testStrFuncs = {
            {"fail", "lux_testFail"},
            {"skip", "lux_testSkip"},
            {"log",  "lux_testLog"},
        };
        auto tsIt = testStrFuncs.find(funcName);
        if (tsIt != testStrFuncs.end()) {
            std::vector<llvm::Value*> tArgs;
            if (auto* argList = ctx->argList())
                for (auto* e : argList->expression())
                    tArgs.push_back(std::any_cast<llvm::Value*>(visit(e)));
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
            auto* voidTy  = llvm::Type::getVoidTy(*context_);
            auto* strPtr = builder_->CreateExtractValue(tArgs[0], 0);
            auto* strLen = builder_->CreateExtractValue(tArgs[0], 1);
            auto callee = declareBuiltin(tsIt->second, voidTy, {ptrTy, usizeTy});
            builder_->CreateCall(callee, {strPtr, strLen});
            return {};
        }
    }

    // ── std::net — void functions (close, setTimeout) ─────────────────
    if (funcName == "close") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList())
            for (auto* e : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(e)));
        auto* i32Ty  = llvm::Type::getInt32Ty(*context_);
        auto* voidTy = llvm::Type::getVoidTy(*context_);
        auto* fd = builder_->CreateIntCast(args[0], i32Ty, true);
        auto callee = declareBuiltin("lux_netClose", voidTy, {i32Ty});
        builder_->CreateCall(callee, {fd});
        return {};
    }
    if (funcName == "setTimeout") {
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList())
            for (auto* e : argList->expression())
                args.push_back(std::any_cast<llvm::Value*>(visit(e)));
        auto* i32Ty  = llvm::Type::getInt32Ty(*context_);
        auto* u64Ty  = llvm::Type::getInt64Ty(*context_);
        auto* voidTy = llvm::Type::getVoidTy(*context_);
        auto* fd = builder_->CreateIntCast(args[0], i32Ty, true);
        auto* ms = args[1];
        if (ms->getType() != u64Ty)
            ms = builder_->CreateIntCast(ms, u64Ty, false);
        auto callee = declareBuiltin("lux_netSetTimeout", voidTy, {i32Ty, u64Ty});
        builder_->CreateCall(callee, {fd, ms});
        return {};
    }

    // ── std::thread — yield: void ───────────────────────────────────────
    if (funcName == "yield") {
        auto callee = declareBuiltin("lux_yield",
            llvm::Type::getVoidTy(*context_), {});
        builder_->CreateCall(callee, {});
        return {};
    }

    // ── std::fs — writeBytes: (string, Vec<uint8>) → void ───────────────
    if (funcName == "writeBytes") {
        std::vector<llvm::Value*> wbArgs;
        if (auto* argList = ctx->argList())
            for (auto* e : argList->expression())
                wbArgs.push_back(std::any_cast<llvm::Value*>(visit(e)));
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* voidTy  = llvm::Type::getVoidTy(*context_);
        auto* vecTy   = getOrCreateVecStructType();
        auto* pathPtr = builder_->CreateExtractValue(wbArgs[0], 0, "wb_path_ptr");
        auto* pathLen = builder_->CreateExtractValue(wbArgs[0], 1, "wb_path_len");
        auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, "wb_vec_arg");
        builder_->CreateStore(wbArgs[1], vecArgAlloca);
        auto callee = declareBuiltin("lux_writeFileBytes", voidTy,
            {ptrTy, usizeTy, ptrTy});
        builder_->CreateCall(callee, {pathPtr, pathLen, vecArgAlloca});
        return {};
    }

    // Collect argument values and their AST expressions
    std::vector<llvm::Value*> args;
    std::vector<LuxParser::ExpressionContext*> argExprs;
    if (auto* argList = ctx->argList()) {
        argExprs = argList->expression();
        for (auto* exprCtx : argExprs) {
            auto val = visit(exprCtx);
            args.push_back(std::any_cast<llvm::Value*>(val));
        }
    }

    for (size_t ai = 0; ai < args.size(); ai++) {
        auto* arg     = args[ai];
        auto* argType = arg->getType();

        // Detect type info via variable lookup
        const TypeInfo* argTypeInfo = nullptr;
        if (auto* load = llvm::dyn_cast<llvm::LoadInst>(arg)) {
            auto* src = load->getPointerOperand();
            for (auto& [vname, info] : locals_) {
                if (info.alloca == src) { argTypeInfo = info.typeInfo; break; }
            }

            // Struct field lookup: LoadInst from GEP into a struct alloca
            if (!argTypeInfo) {
                if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(src)) {
                    auto* base = gep->getPointerOperand();
                    for (auto& [vname, info] : locals_) {
                        if (info.alloca == base &&
                            info.typeInfo && info.typeInfo->kind == TypeKind::Struct) {
                            if (gep->getNumIndices() == 2) {
                                if (auto* ci = llvm::dyn_cast<llvm::ConstantInt>(
                                        gep->getOperand(2))) {
                                    unsigned idx = ci->getZExtValue();
                                    if (idx < info.typeInfo->fields.size())
                                        argTypeInfo = info.typeInfo->fields[idx].typeInfo;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        // String type: extract ptr and len, pass both to builtin
        bool isString = (argTypeInfo && argTypeInfo->kind == TypeKind::String)
                     || (!argTypeInfo && argType->isStructTy());

        if (isString) {
            auto cFuncName = imports_.resolve(funcName, "str");
            if (cFuncName.empty()) {
                std::cerr << "lux: no builtin '" << funcName
                          << "' for type 'str'\n";
                continue;
            }
            auto* strPtr = builder_->CreateExtractValue(arg, 0, "str_ptr");
            auto* strLen = builder_->CreateExtractValue(arg, 1, "str_len");
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
            auto callee   = declareBuiltin(cFuncName,
                                           llvm::Type::getVoidTy(*context_),
                                           { ptrTy, usizeTy });
            builder_->CreateCall(callee, { strPtr, strLen });
            continue;
        }

        std::string suffix;
        bool isCharLit = (ai < argExprs.size()) &&
            dynamic_cast<LuxParser::CharLitExprContext*>(argExprs[ai]) != nullptr;

        if (argTypeInfo) {
            suffix = argTypeInfo->builtinSuffix;
        } else if (isCharLit)                 { suffix = "char"; }
        else if (argType->isIntegerTy(1))     { suffix = "bool"; }
        else if (argType->isIntegerTy(8))     { suffix = "i8"; }
        else if (argType->isIntegerTy(16))    { suffix = "i16"; }
        else if (argType->isIntegerTy(32))    { suffix = "i32"; }
        else if (argType->isIntegerTy(64))    { suffix = "i64"; }
        else if (argType->isIntegerTy(128))   { suffix = "i128"; }
        else if (argType->isFloatTy())        { suffix = "f32"; }
        else if (argType->isDoubleTy())       { suffix = "f64"; }
        else                                  { suffix = "i32"; }
        auto  cFuncName = imports_.resolve(funcName, suffix);

        if (cFuncName.empty()) {
            std::cerr << "lux: no builtin '" << funcName
                      << "' for type '" << suffix << "'\n";
            continue;
        }

        // If the actual type is wider than what the C builtin expects (e.g.
        // intinf/i256 → i128), truncate the value before the call.
        llvm::Value* callArg = arg;
        llvm::Type*  paramType = argType;
        if (argType->isIntegerTy(256)) {
            // Untyped integer literal: match suffix to determine target width
            if (suffix == "i128")
                paramType = llvm::Type::getInt128Ty(*context_);
            else if (suffix == "i64")
                paramType = llvm::Type::getInt64Ty(*context_);
            else
                paramType = llvm::Type::getInt32Ty(*context_);
            callArg = builder_->CreateTrunc(arg, paramType);
        }

        auto callee = declareBuiltin(cFuncName,
                                     llvm::Type::getVoidTy(*context_),
                                     { paramType });
        builder_->CreateCall(callee, { callArg });
    }

    return {};
}

std::any IRGen::visitReturnStmt(LuxParser::ReturnStmtContext* ctx) {
    // Handle bare `ret;` in void functions
    if (!ctx->expression()) {
        emitAllCleanups();
        builder_->CreateRetVoid();
        return {};
    }

    auto  val    = visit(ctx->expression());
    auto* retVal = std::any_cast<llvm::Value*>(val);

    // Cast to the function's return type if needed
    auto* expectedTy = builder_->getCurrentFunctionReturnType();
    if (retVal->getType() != expectedTy) {
        if (retVal->getType()->isIntegerTy() && expectedTy->isIntegerTy()) {
            retVal = builder_->CreateIntCast(retVal, expectedTy, true, "ret_cast");
        } else if (retVal->getType()->isFloatingPointTy() && expectedTy->isFloatingPointTy()) {
            if (retVal->getType()->getPrimitiveSizeInBits() > expectedTy->getPrimitiveSizeInBits())
                retVal = builder_->CreateFPTrunc(retVal, expectedTy, "ret_trunc");
            else
                retVal = builder_->CreateFPExt(retVal, expectedTy, "ret_ext");
        }
    }

    // Emit deferred and auto cleanups before returning.
    // If returning a local collection by value, skip its auto-free.
    std::string skipVar;
    if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(ctx->expression())) {
        skipVar = ident->IDENTIFIER()->getText();
    }
    emitAllCleanups(skipVar);

    builder_->CreateRet(retVal);
    return {};
}

std::any IRGen::visitIfStmt(LuxParser::IfStmtContext* ctx) {
    auto elseIfs = ctx->elseIfClause();
    auto* elseClause = ctx->elseClause();

    // Total branches: 1 (if) + N (else if) + optional else
    size_t totalBranches = 1 + elseIfs.size();

    // Create all basic blocks upfront
    std::vector<llvm::BasicBlock*> condBlocks;   // condition eval blocks
    std::vector<llvm::BasicBlock*> bodyBlocks;    // body blocks
    auto* mergeBlock = llvm::BasicBlock::Create(*context_, "if.end", currentFunction_);

    // First body block (the if-body)
    bodyBlocks.push_back(
        llvm::BasicBlock::Create(*context_, "if.then", currentFunction_));

    // Else-if condition + body blocks
    for (size_t i = 0; i < elseIfs.size(); i++) {
        condBlocks.push_back(
            llvm::BasicBlock::Create(*context_, "elif.cond", currentFunction_));
        bodyBlocks.push_back(
            llvm::BasicBlock::Create(*context_, "elif.then", currentFunction_));
    }

    // Else body block
    llvm::BasicBlock* elseBlock = nullptr;
    if (elseClause) {
        elseBlock = llvm::BasicBlock::Create(*context_, "if.else", currentFunction_);
    }

    // ── Emit the if-condition ──────────────────────────────────────
    auto* cond = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    if (!cond->getType()->isIntegerTy(1))
        cond = builder_->CreateICmpNE(cond,
            llvm::ConstantInt::get(cond->getType(), 0), "tobool");

    auto* falseTarget = !condBlocks.empty() ? condBlocks[0]
                      : elseBlock           ? elseBlock
                                            : mergeBlock;
    builder_->CreateCondBr(cond, bodyBlocks[0], falseTarget);

    // ── Emit the if-body ───────────────────────────────────────────
    builder_->SetInsertPoint(bodyBlocks[0]);
    for (auto* stmt : ctx->block()->statement())
        visit(stmt);
    if (!builder_->GetInsertBlock()->getTerminator())
        builder_->CreateBr(mergeBlock);

    // ── Emit else-if chains ────────────────────────────────────────
    for (size_t i = 0; i < elseIfs.size(); i++) {
        // Condition
        builder_->SetInsertPoint(condBlocks[i]);
        auto* eifCond = std::any_cast<llvm::Value*>(
            visit(elseIfs[i]->expression()));
        if (!eifCond->getType()->isIntegerTy(1))
            eifCond = builder_->CreateICmpNE(eifCond,
                llvm::ConstantInt::get(eifCond->getType(), 0), "tobool");

        auto* nextFalse = (i + 1 < condBlocks.size()) ? condBlocks[i + 1]
                        : elseBlock                    ? elseBlock
                                                       : mergeBlock;
        builder_->CreateCondBr(eifCond, bodyBlocks[i + 1], nextFalse);

        // Body
        builder_->SetInsertPoint(bodyBlocks[i + 1]);
        for (auto* stmt : elseIfs[i]->block()->statement())
            visit(stmt);
        if (!builder_->GetInsertBlock()->getTerminator())
            builder_->CreateBr(mergeBlock);
    }

    // ── Emit else body ─────────────────────────────────────────────
    if (elseBlock) {
        builder_->SetInsertPoint(elseBlock);
        for (auto* stmt : elseClause->block()->statement())
            visit(stmt);
        if (!builder_->GetInsertBlock()->getTerminator())
            builder_->CreateBr(mergeBlock);
    }

    // ── Continue at merge ──────────────────────────────────────────
    builder_->SetInsertPoint(mergeBlock);
    return {};
}

std::any IRGen::visitForClassicStmt(LuxParser::ForClassicStmtContext* ctx) {
    // for TYPE NAME = init; cond; update { body }
    auto* typeInfo = resolveTypeInfo(ctx->typeSpec());
    auto* varType  = typeInfo->toLLVMType(*context_, module_->getDataLayout());
    auto  varName  = ctx->IDENTIFIER()->getText();

    auto exprs = ctx->expression();

    // Init
    auto* initVal = std::any_cast<llvm::Value*>(visit(exprs[0]));
    if (initVal->getType() != varType)
        initVal = builder_->CreateIntCast(initVal, varType, typeInfo->isSigned, "cast");
    auto* alloca = builder_->CreateAlloca(varType, nullptr, varName);
    builder_->CreateStore(initVal, alloca);
    locals_[varName] = {alloca, typeInfo, 0};

    // Basic blocks
    auto* condBB   = llvm::BasicBlock::Create(*context_, "for.cond", currentFunction_);
    auto* bodyBB   = llvm::BasicBlock::Create(*context_, "for.body", currentFunction_);
    auto* updateBB = llvm::BasicBlock::Create(*context_, "for.update", currentFunction_);
    auto* endBB    = llvm::BasicBlock::Create(*context_, "for.end", currentFunction_);

    loopStack_.push_back({endBB, updateBB});

    builder_->CreateBr(condBB);

    // Condition
    builder_->SetInsertPoint(condBB);
    auto* cond = std::any_cast<llvm::Value*>(visit(exprs[1]));
    if (!cond->getType()->isIntegerTy(1))
        cond = builder_->CreateICmpNE(cond,
            llvm::ConstantInt::get(cond->getType(), 0), "tobool");
    builder_->CreateCondBr(cond, bodyBB, endBB);

    // Body
    builder_->SetInsertPoint(bodyBB);
    for (auto* stmt : ctx->block()->statement())
        visit(stmt);
    if (!builder_->GetInsertBlock()->getTerminator())
        builder_->CreateBr(updateBB);

    // Update
    builder_->SetInsertPoint(updateBB);
    visit(exprs[2]);
    builder_->CreateBr(condBB);

    loopStack_.pop_back();
    locals_.erase(varName);

    builder_->SetInsertPoint(endBB);
    return {};
}

std::any IRGen::visitForInStmt(LuxParser::ForInStmtContext* ctx) {
    auto* typeInfo = resolveTypeInfo(ctx->typeSpec());
    auto* varType  = typeInfo->toLLVMType(*context_, module_->getDataLayout());
    auto  varName  = ctx->IDENTIFIER()->getText();
    auto* iterExpr = ctx->expression();

    // Check if iterating over a range (..  or ..=)
    bool isRange = dynamic_cast<LuxParser::RangeExprContext*>(iterExpr) ||
                   dynamic_cast<LuxParser::RangeInclExprContext*>(iterExpr);
    bool isInclusive = dynamic_cast<LuxParser::RangeInclExprContext*>(iterExpr) != nullptr;

    if (isRange) {
        // Range iteration: for TYPE i in start..end  or  start..=end
        llvm::Value* startVal;
        llvm::Value* endVal;

        if (isInclusive) {
            auto* rng = dynamic_cast<LuxParser::RangeInclExprContext*>(iterExpr);
            startVal = std::any_cast<llvm::Value*>(visit(rng->expression(0)));
            endVal   = std::any_cast<llvm::Value*>(visit(rng->expression(1)));
        } else {
            auto* rng = dynamic_cast<LuxParser::RangeExprContext*>(iterExpr);
            startVal = std::any_cast<llvm::Value*>(visit(rng->expression(0)));
            endVal   = std::any_cast<llvm::Value*>(visit(rng->expression(1)));
        }

        // Cast to target type
        if (startVal->getType() != varType)
            startVal = builder_->CreateIntCast(startVal, varType, typeInfo->isSigned, "cast");
        if (endVal->getType() != varType)
            endVal = builder_->CreateIntCast(endVal, varType, typeInfo->isSigned, "cast");

        auto* alloca = builder_->CreateAlloca(varType, nullptr, varName);
        builder_->CreateStore(startVal, alloca);
        locals_[varName] = {alloca, typeInfo, 0};

        auto* condBB   = llvm::BasicBlock::Create(*context_, "for.cond", currentFunction_);
        auto* bodyBB   = llvm::BasicBlock::Create(*context_, "for.body", currentFunction_);
        auto* updateBB = llvm::BasicBlock::Create(*context_, "for.update", currentFunction_);
        auto* endBB    = llvm::BasicBlock::Create(*context_, "for.end", currentFunction_);

        loopStack_.push_back({endBB, updateBB});

        builder_->CreateBr(condBB);

        // Condition: i < end  (exclusive) or  i <= end (inclusive)
        builder_->SetInsertPoint(condBB);
        auto* cur = builder_->CreateLoad(varType, alloca, varName);
        llvm::Value* cond;
        if (isInclusive)
            cond = typeInfo->isSigned
                 ? builder_->CreateICmpSLE(cur, endVal, "cmp")
                 : builder_->CreateICmpULE(cur, endVal, "cmp");
        else
            cond = typeInfo->isSigned
                 ? builder_->CreateICmpSLT(cur, endVal, "cmp")
                 : builder_->CreateICmpULT(cur, endVal, "cmp");
        builder_->CreateCondBr(cond, bodyBB, endBB);

        // Body
        builder_->SetInsertPoint(bodyBB);
        for (auto* stmt : ctx->block()->statement())
            visit(stmt);
        if (!builder_->GetInsertBlock()->getTerminator())
            builder_->CreateBr(updateBB);

        // Update: i++
        builder_->SetInsertPoint(updateBB);
        auto* curVal = builder_->CreateLoad(varType, alloca, "cur");
        auto* next = builder_->CreateAdd(curVal,
            llvm::ConstantInt::get(varType, 1), "inc");
        builder_->CreateStore(next, alloca);
        builder_->CreateBr(condBB);

        loopStack_.pop_back();
        locals_.erase(varName);

        builder_->SetInsertPoint(endBB);
    } else {
        // Check if iterating over a variadic parameter
        std::string iterName;
        if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(iterExpr))
            iterName = ident->IDENTIFIER()->getText();

        auto vpIt = variadicParams_.find(iterName);
        bool isVariadicParam = !iterName.empty() && vpIt != variadicParams_.end();

        if (isVariadicParam) {
            // Variadic param iteration: for TYPE x in variadicParam
            auto* ptrTy = llvm::PointerType::getUnqual(*context_);
            auto* i64Ty = llvm::Type::getInt64Ty(*context_);
            auto* elemTy = vpIt->second.elementType->toLLVMType(*context_, module_->getDataLayout());

            auto* dataPtr = builder_->CreateLoad(ptrTy, vpIt->second.dataPtr, "var_data");
            auto* varLen  = builder_->CreateLoad(i64Ty, vpIt->second.lenAlloca, "var_len");

            auto* idxAlloca = builder_->CreateAlloca(i64Ty, nullptr, "idx");
            builder_->CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);

            auto* elemAlloca = builder_->CreateAlloca(varType, nullptr, varName);
            locals_[varName] = {elemAlloca, typeInfo, 0};

            auto* condBB   = llvm::BasicBlock::Create(*context_, "for.cond", currentFunction_);
            auto* bodyBB   = llvm::BasicBlock::Create(*context_, "for.body", currentFunction_);
            auto* updateBB = llvm::BasicBlock::Create(*context_, "for.update", currentFunction_);
            auto* endBB    = llvm::BasicBlock::Create(*context_, "for.end", currentFunction_);

            loopStack_.push_back({endBB, updateBB});
            builder_->CreateBr(condBB);

            // Condition: idx < len
            builder_->SetInsertPoint(condBB);
            auto* idx  = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
            auto* cond = builder_->CreateICmpULT(idx, varLen, "cmp");
            builder_->CreateCondBr(cond, bodyBB, endBB);

            // Body: load element via GEP on data pointer
            builder_->SetInsertPoint(bodyBB);
            auto* curIdx = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
            auto* gep = builder_->CreateGEP(elemTy, dataPtr, curIdx, "elem_ptr");
            auto* elem = builder_->CreateLoad(varType, gep, "elem");
            builder_->CreateStore(elem, elemAlloca);

            for (auto* stmt : ctx->block()->statement())
                visit(stmt);
            if (!builder_->GetInsertBlock()->getTerminator())
                builder_->CreateBr(updateBB);

            // Update: idx++
            builder_->SetInsertPoint(updateBB);
            auto* curIdxUpd = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
            auto* nextIdx   = builder_->CreateAdd(curIdxUpd,
                llvm::ConstantInt::get(i64Ty, 1), "inc");
            builder_->CreateStore(nextIdx, idxAlloca);
            builder_->CreateBr(condBB);

            loopStack_.pop_back();
            locals_.erase(varName);

            builder_->SetInsertPoint(endBB);
        } else {
        // Detect if the iterable is a Vec<T> (Extended type)
        const TypeInfo* iterableTI = nullptr;
        llvm::AllocaInst* vecAlloca = nullptr;
        if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(iterExpr)) {
            auto it = locals_.find(ident->IDENTIFIER()->getText());
            if (it != locals_.end()) {
                iterableTI = it->second.typeInfo;
                vecAlloca  = it->second.alloca;
            }
        }

        // Support method calls that return Vec (e.g. map.keys(), map.values(), set.values())
        if (!vecAlloca) {
            if (auto* mc = dynamic_cast<LuxParser::MethodCallExprContext*>(iterExpr)) {
                auto* baseExpr = mc->expression();
                if (auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(baseExpr)) {
                    auto recvIt = locals_.find(identBase->IDENTIFIER()->getText());
                    if (recvIt != locals_.end() &&
                        recvIt->second.typeInfo->kind == TypeKind::Extended) {
                        auto mName = mc->IDENTIFIER()->getText();
                        if (mName == "keys" || mName == "values") {
                            auto* vecVal = std::any_cast<llvm::Value*>(visit(mc));
                            auto* vecTy  = getOrCreateVecStructType();
                            vecAlloca = builder_->CreateAlloca(vecTy, nullptr,
                                                               mName + "_tmp");
                            builder_->CreateStore(vecVal, vecAlloca);
                            iterableTI = recvIt->second.typeInfo;
                        }
                    }
                }
            }
        }

        bool isVec = iterableTI && iterableTI->kind == TypeKind::Extended;

        if (isVec) {
            // Vec iteration: for TYPE x in vec
            // Determine suffix: from Vec's element type, or from the loop variable type
            std::string suffix;
            if (iterableTI->elementType)
                suffix = getVecSuffix(iterableTI->elementType);
            else
                suffix = typeInfo->builtinSuffix;
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);

            // Get vec length: lux_vec_len_<suffix>(&vec)
            auto lenCallee = declareBuiltin("lux_vec_len_" + suffix, usizeTy, {ptrTy});
            auto* vecLen = builder_->CreateCall(lenCallee, {vecAlloca}, "vec_len");

            auto* idxType   = usizeTy;
            auto* idxAlloca = builder_->CreateAlloca(idxType, nullptr, "idx");
            builder_->CreateStore(llvm::ConstantInt::get(idxType, 0), idxAlloca);

            auto* elemAlloca = builder_->CreateAlloca(varType, nullptr, varName);
            locals_[varName] = {elemAlloca, typeInfo, 0};

            auto* condBB   = llvm::BasicBlock::Create(*context_, "for.cond", currentFunction_);
            auto* bodyBB   = llvm::BasicBlock::Create(*context_, "for.body", currentFunction_);
            auto* updateBB = llvm::BasicBlock::Create(*context_, "for.update", currentFunction_);
            auto* endBB    = llvm::BasicBlock::Create(*context_, "for.end", currentFunction_);

            loopStack_.push_back({endBB, updateBB});

            builder_->CreateBr(condBB);

            // Condition: idx < vec.len()
            builder_->SetInsertPoint(condBB);
            auto* idx  = builder_->CreateLoad(idxType, idxAlloca, "idx");
            auto* cond = builder_->CreateICmpULT(idx, vecLen, "cmp");
            builder_->CreateCondBr(cond, bodyBB, endBB);

            // Body: elem = lux_vec_at_<suffix>(&vec, idx)
            builder_->SetInsertPoint(bodyBB);
            auto* curIdx = builder_->CreateLoad(idxType, idxAlloca, "idx");
            auto atCallee = declareBuiltin("lux_vec_at_" + suffix, varType, {ptrTy, usizeTy});
            auto* elem = builder_->CreateCall(atCallee, {vecAlloca, curIdx}, "elem");
            builder_->CreateStore(elem, elemAlloca);

            for (auto* stmt : ctx->block()->statement())
                visit(stmt);
            if (!builder_->GetInsertBlock()->getTerminator())
                builder_->CreateBr(updateBB);

            // Update: idx++
            builder_->SetInsertPoint(updateBB);
            auto* curIdxUpd = builder_->CreateLoad(idxType, idxAlloca, "idx");
            auto* nextIdx   = builder_->CreateAdd(curIdxUpd,
                llvm::ConstantInt::get(idxType, 1), "inc");
            builder_->CreateStore(nextIdx, idxAlloca);
            builder_->CreateBr(condBB);

            loopStack_.pop_back();
            locals_.erase(varName);

            builder_->SetInsertPoint(endBB);
        } else {
        // Array iteration: for TYPE x in arr
        auto* arrVal = std::any_cast<llvm::Value*>(visit(iterExpr));

        // Get array length from the source variable
        unsigned arrLen = 0;
        if (auto* arrTy = llvm::dyn_cast<llvm::ArrayType>(arrVal->getType()))
            arrLen = arrTy->getNumElements();

        // Find the source alloca for indexed access
        llvm::AllocaInst* arrAlloca = nullptr;
        if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(iterExpr)) {
            auto it = locals_.find(ident->IDENTIFIER()->getText());
            if (it != locals_.end())
                arrAlloca = it->second.alloca;
        }

        auto* idxType = llvm::Type::getInt64Ty(*context_);
        auto* idxAlloca = builder_->CreateAlloca(idxType, nullptr, "idx");
        builder_->CreateStore(llvm::ConstantInt::get(idxType, 0), idxAlloca);

        auto* elemAlloca = builder_->CreateAlloca(varType, nullptr, varName);
        locals_[varName] = {elemAlloca, typeInfo, 0};

        auto* condBB   = llvm::BasicBlock::Create(*context_, "for.cond", currentFunction_);
        auto* bodyBB   = llvm::BasicBlock::Create(*context_, "for.body", currentFunction_);
        auto* updateBB = llvm::BasicBlock::Create(*context_, "for.update", currentFunction_);
        auto* endBB    = llvm::BasicBlock::Create(*context_, "for.end", currentFunction_);

        loopStack_.push_back({endBB, updateBB});

        builder_->CreateBr(condBB);

        // Condition: idx < arrLen
        builder_->SetInsertPoint(condBB);
        auto* idx = builder_->CreateLoad(idxType, idxAlloca, "idx");
        auto* cond = builder_->CreateICmpULT(idx,
            llvm::ConstantInt::get(idxType, arrLen), "cmp");
        builder_->CreateCondBr(cond, bodyBB, endBB);

        // Body: load element from array
        builder_->SetInsertPoint(bodyBB);
        auto* curIdx = builder_->CreateLoad(idxType, idxAlloca, "idx");
        llvm::Value* elemPtr;
        if (arrAlloca) {
            elemPtr = builder_->CreateInBoundsGEP(
                arrAlloca->getAllocatedType(), arrAlloca,
                {llvm::ConstantInt::get(idxType, 0), curIdx}, "elem_ptr");
        } else {
            elemPtr = builder_->CreateInBoundsGEP(
                varType, arrVal, {curIdx}, "elem_ptr");
        }
        auto* elem = builder_->CreateLoad(varType, elemPtr, "elem");
        builder_->CreateStore(elem, elemAlloca);

        for (auto* stmt : ctx->block()->statement())
            visit(stmt);
        if (!builder_->GetInsertBlock()->getTerminator())
            builder_->CreateBr(updateBB);

        // Update: idx++
        builder_->SetInsertPoint(updateBB);
        auto* curIdxUpdate = builder_->CreateLoad(idxType, idxAlloca, "idx");
        auto* nextIdx = builder_->CreateAdd(curIdxUpdate,
            llvm::ConstantInt::get(idxType, 1), "inc");
        builder_->CreateStore(nextIdx, idxAlloca);
        builder_->CreateBr(condBB);

        loopStack_.pop_back();
        locals_.erase(varName);

        builder_->SetInsertPoint(endBB);
        }
        } // close variadic else
    }

    return {};
}

std::any IRGen::visitBreakStmt(LuxParser::BreakStmtContext* /*ctx*/) {
    if (!loopStack_.empty())
        builder_->CreateBr(loopStack_.back().breakTarget);
    return {};
}

std::any IRGen::visitContinueStmt(LuxParser::ContinueStmtContext* /*ctx*/) {
    if (!loopStack_.empty())
        builder_->CreateBr(loopStack_.back().continueTarget);
    return {};
}

std::any IRGen::visitLoopStmt(LuxParser::LoopStmtContext* ctx) {
    auto* bodyBB = llvm::BasicBlock::Create(*context_, "loop.body", currentFunction_);
    auto* endBB  = llvm::BasicBlock::Create(*context_, "loop.end", currentFunction_);

    loopStack_.push_back({endBB, bodyBB});

    builder_->CreateBr(bodyBB);

    builder_->SetInsertPoint(bodyBB);
    for (auto* stmt : ctx->block()->statement())
        visit(stmt);
    if (!builder_->GetInsertBlock()->getTerminator())
        builder_->CreateBr(bodyBB);

    loopStack_.pop_back();

    builder_->SetInsertPoint(endBB);
    return {};
}

std::any IRGen::visitSwitchStmt(LuxParser::SwitchStmtContext* ctx) {
    // Evaluate the switch expression
    auto* switchVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    auto* mergeBB = llvm::BasicBlock::Create(*context_, "switch.end", currentFunction_);

    // Default block: either the user-defined default or merge
    llvm::BasicBlock* defaultBB = mergeBB;
    if (ctx->defaultClause()) {
        defaultBB = llvm::BasicBlock::Create(*context_, "switch.default", currentFunction_);
    }

    auto caseClauses = ctx->caseClause();
    auto* switchInst = builder_->CreateSwitch(switchVal, defaultBB,
                                              static_cast<unsigned>(caseClauses.size()));

    // Emit each case
    for (auto* cc : caseClauses) {
        auto* caseBB = llvm::BasicBlock::Create(*context_, "switch.case", currentFunction_);

        // Each case can have multiple values: case 1, 2, 3 { ... }
        auto caseExprs = cc->expression();
        for (auto* caseExpr : caseExprs) {
            auto* caseVal = std::any_cast<llvm::Value*>(visit(caseExpr));
            // Promote to match switch value width
            if (caseVal->getType() != switchVal->getType())
                caseVal = builder_->CreateIntCast(caseVal, switchVal->getType(), true);
            auto* constVal = llvm::dyn_cast<llvm::ConstantInt>(caseVal);
            switchInst->addCase(constVal, caseBB);
        }

        // Emit case body
        builder_->SetInsertPoint(caseBB);
        for (auto* stmt : cc->block()->statement())
            visit(stmt);
        if (!builder_->GetInsertBlock()->getTerminator())
            builder_->CreateBr(mergeBB);
    }

    // Emit default body
    if (ctx->defaultClause()) {
        builder_->SetInsertPoint(defaultBB);
        for (auto* stmt : ctx->defaultClause()->block()->statement())
            visit(stmt);
        if (!builder_->GetInsertBlock()->getTerminator())
            builder_->CreateBr(mergeBB);
    }

    builder_->SetInsertPoint(mergeBB);
    return {};
}

std::any IRGen::visitWhileStmt(LuxParser::WhileStmtContext* ctx) {
    auto* condBB = llvm::BasicBlock::Create(*context_, "while.cond", currentFunction_);
    auto* bodyBB = llvm::BasicBlock::Create(*context_, "while.body", currentFunction_);
    auto* endBB  = llvm::BasicBlock::Create(*context_, "while.end", currentFunction_);

    loopStack_.push_back({endBB, condBB});

    builder_->CreateBr(condBB);

    // Condition
    builder_->SetInsertPoint(condBB);
    auto* cond = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    if (!cond->getType()->isIntegerTy(1))
        cond = builder_->CreateICmpNE(cond,
            llvm::ConstantInt::get(cond->getType(), 0), "tobool");
    builder_->CreateCondBr(cond, bodyBB, endBB);

    // Body
    builder_->SetInsertPoint(bodyBB);
    for (auto* stmt : ctx->block()->statement())
        visit(stmt);
    if (!builder_->GetInsertBlock()->getTerminator())
        builder_->CreateBr(condBB);

    loopStack_.pop_back();

    builder_->SetInsertPoint(endBB);
    return {};
}

std::any IRGen::visitDoWhileStmt(LuxParser::DoWhileStmtContext* ctx) {
    auto* bodyBB = llvm::BasicBlock::Create(*context_, "do.body", currentFunction_);
    auto* condBB = llvm::BasicBlock::Create(*context_, "do.cond", currentFunction_);
    auto* endBB  = llvm::BasicBlock::Create(*context_, "do.end", currentFunction_);

    loopStack_.push_back({endBB, condBB});

    builder_->CreateBr(bodyBB);

    // Body (executes at least once)
    builder_->SetInsertPoint(bodyBB);
    for (auto* stmt : ctx->block()->statement())
        visit(stmt);
    if (!builder_->GetInsertBlock()->getTerminator())
        builder_->CreateBr(condBB);

    // Condition
    builder_->SetInsertPoint(condBB);
    auto* cond = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    if (!cond->getType()->isIntegerTy(1))
        cond = builder_->CreateICmpNE(cond,
            llvm::ConstantInt::get(cond->getType(), 0), "tobool");
    builder_->CreateCondBr(cond, bodyBB, endBB);

    loopStack_.pop_back();

    builder_->SetInsertPoint(endBB);
    return {};
}

std::any IRGen::visitIntLitExpr(LuxParser::IntLitExprContext* ctx) {
    auto text = ctx->INT_LIT()->getText();
    llvm::APInt ap(256, text, 10);
    auto* ty = llvm::Type::getIntNTy(*context_, 256);
    return static_cast<llvm::Value*>(llvm::ConstantInt::get(ty, ap));
}

std::any IRGen::visitFloatLitExpr(LuxParser::FloatLitExprContext* ctx) {
    double v = std::stod(ctx->FLOAT_LIT()->getText());
    return static_cast<llvm::Value*>(
        llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), v));
}

std::any IRGen::visitBoolLitExpr(LuxParser::BoolLitExprContext* ctx) {
    bool v = ctx->BOOL_LIT()->getText() == "true";
    return static_cast<llvm::Value*>(
        llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), v ? 1 : 0));
}

std::any IRGen::visitCharLitExpr(LuxParser::CharLitExprContext* ctx) {
    auto raw = ctx->CHAR_LIT()->getText();
    auto inner = raw.substr(1, raw.size() - 2);
    uint8_t ch;
    if (inner.size() >= 2 && inner[0] == '\\') {
        switch (inner[1]) {
        case 'n':  ch = '\n'; break;
        case 'r':  ch = '\r'; break;
        case 't':  ch = '\t'; break;
        case '\\': ch = '\\'; break;
        case '\'': ch = '\''; break;
        case '"':  ch = '"';  break;
        case '0':  ch = '\0'; break;
        case 'a':  ch = '\a'; break;
        case 'b':  ch = '\b'; break;
        case 'f':  ch = '\f'; break;
        case 'v':  ch = '\v'; break;
        case 'x': {
            auto hex = inner.substr(2);
            ch = static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
            break;
        }
        default:
            std::cerr << "lux: unknown escape sequence '\\" << inner[1] << "'\n";
            ch = inner[1];
            break;
        }
    } else {
        ch = static_cast<uint8_t>(inner[0]);
    }
    return static_cast<llvm::Value*>(
        llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context_), ch));
}

std::any IRGen::visitStrLitExpr(LuxParser::StrLitExprContext* ctx) {
    auto raw = ctx->STR_LIT()->getText();
    auto escaped = raw.substr(1, raw.size() - 2);

    // Process escape sequences
    std::string str;
    str.reserve(escaped.size());
    for (size_t i = 0; i < escaped.size(); ++i) {
        if (escaped[i] == '\\' && i + 1 < escaped.size()) {
            char next = escaped[++i];
            switch (next) {
                case 'n':  str += '\n'; break;
                case 'r':  str += '\r'; break;
                case 't':  str += '\t'; break;
                case '\\': str += '\\'; break;
                case '"':  str += '"';  break;
                case '\'': str += '\''; break;
                case '0':  str += '\0'; break;
                case 'a':  str += '\a'; break;
                case 'b':  str += '\b'; break;
                case 'f':  str += '\f'; break;
                case 'v':  str += '\v'; break;
                case 'x':
                    if (i + 2 < escaped.size()) {
                        auto hex = escaped.substr(i + 1, 2);
                        str += static_cast<char>(std::stoi(hex, nullptr, 16));
                        i += 2;
                    }
                    break;
                default:
                    str += '\\';
                    str += next;
                    break;
            }
        } else {
            str += escaped[i];
        }
    }

    auto* strGlobal = builder_->CreateGlobalString(str, ".str", 0, module_);

    auto* strTy   = typeRegistry_.lookup("string")
                        ->toLLVMType(*context_, module_->getDataLayout());
    auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
    auto* len     = llvm::ConstantInt::get(usizeTy, str.size());

    llvm::Value* strStruct = llvm::UndefValue::get(strTy);
    strStruct = builder_->CreateInsertValue(strStruct, strGlobal, 0, "str_ptr");
    strStruct = builder_->CreateInsertValue(strStruct, len,       1, "str_len");
    return static_cast<llvm::Value*>(strStruct);
}

std::any IRGen::visitCStrLitExpr(LuxParser::CStrLitExprContext* ctx) {
    auto raw = ctx->C_STR_LIT()->getText();
    // Strip the c"..." wrapper: skip 'c"' at start and '"' at end
    auto escaped = raw.substr(2, raw.size() - 3);

    // Process escape sequences (same as string literals)
    std::string str;
    str.reserve(escaped.size());
    for (size_t i = 0; i < escaped.size(); ++i) {
        if (escaped[i] == '\\' && i + 1 < escaped.size()) {
            char next = escaped[++i];
            switch (next) {
                case 'n':  str += '\n'; break;
                case 'r':  str += '\r'; break;
                case 't':  str += '\t'; break;
                case '\\': str += '\\'; break;
                case '"':  str += '"';  break;
                case '\'': str += '\''; break;
                case '0':  str += '\0'; break;
                case 'a':  str += '\a'; break;
                case 'b':  str += '\b'; break;
                case 'f':  str += '\f'; break;
                case 'v':  str += '\v'; break;
                case 'x':
                    if (i + 2 < escaped.size()) {
                        auto hex = escaped.substr(i + 1, 2);
                        str += static_cast<char>(std::stoi(hex, nullptr, 16));
                        i += 2;
                    }
                    break;
                default:
                    str += '\\';
                    str += next;
                    break;
            }
        } else {
            str += escaped[i];
        }
    }

    // Emit as a null-terminated global constant, return pointer (ptr)
    auto* cstrGlobal = builder_->CreateGlobalString(str, ".cstr", 0, module_);
    return static_cast<llvm::Value*>(cstrGlobal);
}

std::any IRGen::visitIdentExpr(LuxParser::IdentExprContext* ctx) {
    auto name = ctx->IDENTIFIER()->getText();
    auto it   = locals_.find(name);
    if (it != locals_.end()) {
        auto* alloca = it->second.alloca;
        return static_cast<llvm::Value*>(
            builder_->CreateLoad(alloca->getAllocatedType(), alloca, name));
    }

    // ── Variadic parameter: return data pointer ─────────────────────────
    auto vit = variadicParams_.find(name);
    if (vit != variadicParams_.end()) {
        auto* ptrTy = llvm::PointerType::getUnqual(*context_);
        return static_cast<llvm::Value*>(
            builder_->CreateLoad(ptrTy, vit->second.dataPtr, name));
    }

    // ── std::math constants ─────────────────────────────────────────────
    if (imports_.isImported(name)) {
        auto* f64Ty = llvm::Type::getDoubleTy(*context_);
        auto* i32Ty = llvm::Type::getInt32Ty(*context_);
        auto* i64Ty = llvm::Type::getInt64Ty(*context_);

        if (name == "PI")
            return static_cast<llvm::Value*>(
                llvm::ConstantFP::get(f64Ty, 3.14159265358979323846));
        if (name == "E")
            return static_cast<llvm::Value*>(
                llvm::ConstantFP::get(f64Ty, 2.71828182845904523536));
        if (name == "TAU")
            return static_cast<llvm::Value*>(
                llvm::ConstantFP::get(f64Ty, 6.28318530717958647692));
        if (name == "INF")
            return static_cast<llvm::Value*>(
                llvm::ConstantFP::getInfinity(f64Ty));
        if (name == "NAN")
            return static_cast<llvm::Value*>(
                llvm::ConstantFP::getNaN(f64Ty));
        if (name == "INT32_MAX")
            return static_cast<llvm::Value*>(
                llvm::ConstantInt::get(i32Ty, 2147483647));
        if (name == "INT32_MIN")
            return static_cast<llvm::Value*>(
                llvm::ConstantInt::get(i32Ty, (uint64_t)(int32_t)-2147483648, true));
        if (name == "INT64_MAX")
            return static_cast<llvm::Value*>(
                llvm::ConstantInt::get(i64Ty, 9223372036854775807ULL));
        if (name == "INT64_MIN")
            return static_cast<llvm::Value*>(
                llvm::ConstantInt::get(i64Ty, (uint64_t)(-9223372036854775807LL - 1), true));
        if (name == "UINT32_MAX")
            return static_cast<llvm::Value*>(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 4294967295U));
        if (name == "UINT64_MAX")
            return static_cast<llvm::Value*>(
                llvm::ConstantInt::get(i64Ty, 18446744073709551615ULL));
    }

    // Check if it's a module-level function (function reference)
    if (auto* fn = module_->getFunction(name)) {
        return static_cast<llvm::Value*>(fn);
    }
    // Try mangled name via callTargetMap_
    {
        auto cit = callTargetMap_.find(name);
        if (cit != callTargetMap_.end()) {
            if (auto* fn = module_->getFunction(cit->second))
                return static_cast<llvm::Value*>(fn);
        }
    }

    // ── C enum constants ────────────────────────────────────────────────
    {
        auto eit = cEnumConstants_.find(name);
        if (eit != cEnumConstants_.end()) {
            return static_cast<llvm::Value*>(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_),
                                       static_cast<uint64_t>(eit->second), true));
        }
    }

    // ── C struct literal macros (e.g. RAYWHITE → Color{245,245,245,255}) ─
    {
        auto sit = cStructMacros_.find(name);
        if (sit != cStructMacros_.end()) {
            const auto* sm = sit->second;
            auto* structTy = llvm::StructType::getTypeByName(*context_,
                                                              sm->structType);
            if (structTy) {
                std::vector<llvm::Constant*> fields;
                unsigned numElements = structTy->getNumElements();
                for (unsigned i = 0; i < numElements && i < sm->fieldValues.size(); ++i) {
                    auto* fieldTy = structTy->getElementType(i);
                    fields.push_back(llvm::ConstantInt::get(
                        fieldTy, static_cast<uint64_t>(sm->fieldValues[i]),
                        /* isSigned */ true));
                }
                // Pad remaining fields with zero if macro has fewer values
                for (unsigned i = static_cast<unsigned>(sm->fieldValues.size());
                     i < numElements; ++i) {
                    fields.push_back(llvm::Constant::getNullValue(
                        structTy->getElementType(i)));
                }
                return static_cast<llvm::Value*>(
                    llvm::ConstantStruct::get(structTy, fields));
            }
        }
    }

    // ── C global variables ──────────────────────────────────────────────
    {
        auto git = cGlobals_.find(name);
        if (git != cGlobals_.end()) {
            auto& entry = git->second;
            auto* loadedTy = entry.global->getValueType();
            return static_cast<llvm::Value*>(
                builder_->CreateLoad(loadedTy, entry.global, name));
        }
    }

    std::cerr << "lux: undefined variable '" << name << "'\n";
    return static_cast<llvm::Value*>(
        llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
}

std::any IRGen::visitArrayLitExpr(LuxParser::ArrayLitExprContext* ctx) {
    auto elems = ctx->expression();
    if (elems.empty()) {
        std::cerr << "lux: empty array literal\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    std::vector<llvm::Value*> vals;
    for (auto* e : elems)
        vals.push_back(std::any_cast<llvm::Value*>(visit(e)));

    auto* elemTy = vals[0]->getType();
    auto* arrTy  = llvm::ArrayType::get(elemTy, vals.size());
    llvm::Value* agg = llvm::UndefValue::get(arrTy);
    for (size_t i = 0; i < vals.size(); i++)
        agg = builder_->CreateInsertValue(agg, vals[i], {static_cast<unsigned>(i)});

    return static_cast<llvm::Value*>(agg);
}

std::any IRGen::visitListCompExpr(LuxParser::ListCompExprContext* ctx) {
    auto* typeInfo = resolveTypeInfo(ctx->typeSpec());
    auto* varType  = typeInfo->toLLVMType(*context_, module_->getDataLayout());
    auto  varName  = ctx->IDENTIFIER()->getText();
    auto* iterExpr = ctx->expression(1);

    // Determine if iterating over a range
    bool isRange = dynamic_cast<LuxParser::RangeExprContext*>(iterExpr) ||
                   dynamic_cast<LuxParser::RangeInclExprContext*>(iterExpr);
    bool isInclusive = dynamic_cast<LuxParser::RangeInclExprContext*>(iterExpr) != nullptr;

    bool hasFilter = ctx->IF() != nullptr;

    if (isRange) {
        // Get start and end values
        llvm::Value* startVal;
        llvm::Value* endVal;

        if (isInclusive) {
            auto* rng = dynamic_cast<LuxParser::RangeInclExprContext*>(iterExpr);
            startVal = std::any_cast<llvm::Value*>(visit(rng->expression(0)));
            endVal   = std::any_cast<llvm::Value*>(visit(rng->expression(1)));
        } else {
            auto* rng = dynamic_cast<LuxParser::RangeExprContext*>(iterExpr);
            startVal = std::any_cast<llvm::Value*>(visit(rng->expression(0)));
            endVal   = std::any_cast<llvm::Value*>(visit(rng->expression(1)));
        }

        if (startVal->getType() != varType)
            startVal = builder_->CreateIntCast(startVal, varType, typeInfo->isSigned, "lc.cast");
        if (endVal->getType() != varType)
            endVal = builder_->CreateIntCast(endVal, varType, typeInfo->isSigned, "lc.cast");

        // Compute array size at compile time from constants
        auto* startConst = llvm::dyn_cast<llvm::ConstantInt>(startVal);
        auto* endConst   = llvm::dyn_cast<llvm::ConstantInt>(endVal);

        if (!startConst || !endConst) {
            std::cerr << "lux: list comprehension range bounds must be constants\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        int64_t s = startConst->getSExtValue();
        int64_t e = endConst->getSExtValue();
        uint64_t rangeLen = isInclusive ? (e - s + 1) : (e - s);

        // Allocate the loop variable
        auto* loopVar = builder_->CreateAlloca(varType, nullptr, varName);
        builder_->CreateStore(startVal, loopVar);
        locals_[varName] = { loopVar, typeInfo, 0 };

        if (!hasFilter) {
            // No filter: output array is same size as range
            // Evaluate the value expression for each iteration
            auto* valExpr = ctx->expression(0);

            // First evaluate to get the element type
            auto* firstVal = std::any_cast<llvm::Value*>(visit(valExpr));
            auto* elemTy = firstVal->getType();
            auto* arrTy = llvm::ArrayType::get(elemTy, rangeLen);

            // Allocate array on the stack and fill via loop
            auto* arrAlloca = builder_->CreateAlloca(arrTy, nullptr, "lc.arr");
            auto* i64Ty = llvm::Type::getInt64Ty(*context_);
            auto* idxAlloca = builder_->CreateAlloca(i64Ty, nullptr, "lc.idx");
            builder_->CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);

            // Store the first element (already computed)
            auto* gep0 = builder_->CreateInBoundsGEP(
                arrTy, arrAlloca,
                { llvm::ConstantInt::get(i64Ty, 0),
                  llvm::ConstantInt::get(i64Ty, 0) }, "lc.gep");
            builder_->CreateStore(firstVal, gep0);

            // Store idx=1, loopVar = start+1
            builder_->CreateStore(llvm::ConstantInt::get(i64Ty, 1), idxAlloca);
            auto* nextStart = builder_->CreateAdd(startVal,
                llvm::ConstantInt::get(varType, 1), "lc.next");
            builder_->CreateStore(nextStart, loopVar);

            auto* condBB   = llvm::BasicBlock::Create(*context_, "lc.cond", currentFunction_);
            auto* bodyBB   = llvm::BasicBlock::Create(*context_, "lc.body", currentFunction_);
            auto* endBB    = llvm::BasicBlock::Create(*context_, "lc.end", currentFunction_);

            builder_->CreateBr(condBB);

            // Condition
            builder_->SetInsertPoint(condBB);
            auto* cur = builder_->CreateLoad(varType, loopVar, "lc.cur");
            llvm::Value* cond;
            if (isInclusive)
                cond = typeInfo->isSigned
                     ? builder_->CreateICmpSLE(cur, endVal, "lc.cmp")
                     : builder_->CreateICmpULE(cur, endVal, "lc.cmp");
            else
                cond = typeInfo->isSigned
                     ? builder_->CreateICmpSLT(cur, endVal, "lc.cmp")
                     : builder_->CreateICmpULT(cur, endVal, "lc.cmp");
            builder_->CreateCondBr(cond, bodyBB, endBB);

            // Body
            builder_->SetInsertPoint(bodyBB);
            auto* val = std::any_cast<llvm::Value*>(visit(valExpr));
            auto* idx = builder_->CreateLoad(i64Ty, idxAlloca, "lc.idx");
            auto* gep = builder_->CreateInBoundsGEP(
                arrTy, arrAlloca,
                { llvm::ConstantInt::get(i64Ty, 0), idx }, "lc.gep");
            builder_->CreateStore(val, gep);

            // Update: idx++, loopVar++
            auto* newIdx = builder_->CreateAdd(idx,
                llvm::ConstantInt::get(i64Ty, 1), "lc.idx.inc");
            builder_->CreateStore(newIdx, idxAlloca);
            auto* curVar = builder_->CreateLoad(varType, loopVar, "lc.var");
            auto* nextVar = builder_->CreateAdd(curVar,
                llvm::ConstantInt::get(varType, 1), "lc.var.inc");
            builder_->CreateStore(nextVar, loopVar);
            builder_->CreateBr(condBB);

            builder_->SetInsertPoint(endBB);
            locals_.erase(varName);

            // Load the completed array from the alloca
            auto* result = builder_->CreateLoad(arrTy, arrAlloca, "lc.result");
            return static_cast<llvm::Value*>(result);
        } else {
            // With filter: use max-size array and write index
            auto* valExpr = ctx->expression(0);
            auto* filterExpr = ctx->expression(2);

            // Pre-evaluate to get element type (use start value)
            auto* firstVal = std::any_cast<llvm::Value*>(visit(valExpr));
            auto* elemTy = firstVal->getType();
            auto* arrTy = llvm::ArrayType::get(elemTy, rangeLen);

            auto* arrAlloca = builder_->CreateAlloca(arrTy, nullptr, "lc.arr");
            // Zero-init
            builder_->CreateStore(llvm::Constant::getNullValue(arrTy), arrAlloca);

            auto* i64Ty = llvm::Type::getInt64Ty(*context_);
            auto* writeIdx = builder_->CreateAlloca(i64Ty, nullptr, "lc.widx");
            builder_->CreateStore(llvm::ConstantInt::get(i64Ty, 0), writeIdx);

            // Check first element against filter
            auto* filterVal0 = std::any_cast<llvm::Value*>(visit(filterExpr));
            auto* filterCond0BB = llvm::BasicBlock::Create(*context_, "lc.filt0.then", currentFunction_);
            auto* filterJoin0BB = llvm::BasicBlock::Create(*context_, "lc.filt0.join", currentFunction_);
            builder_->CreateCondBr(filterVal0, filterCond0BB, filterJoin0BB);

            builder_->SetInsertPoint(filterCond0BB);
            auto* wi0 = builder_->CreateLoad(i64Ty, writeIdx, "lc.widx");
            auto* gep0 = builder_->CreateInBoundsGEP(
                arrTy, arrAlloca,
                { llvm::ConstantInt::get(i64Ty, 0), wi0 }, "lc.gep");
            builder_->CreateStore(firstVal, gep0);
            auto* wi0inc = builder_->CreateAdd(wi0,
                llvm::ConstantInt::get(i64Ty, 1), "lc.widx.inc");
            builder_->CreateStore(wi0inc, writeIdx);
            builder_->CreateBr(filterJoin0BB);

            builder_->SetInsertPoint(filterJoin0BB);

            // Continue loop from start+1
            auto* nextStart = builder_->CreateAdd(startVal,
                llvm::ConstantInt::get(varType, 1), "lc.next");
            builder_->CreateStore(nextStart, loopVar);

            auto* condBB = llvm::BasicBlock::Create(*context_, "lc.cond", currentFunction_);
            auto* bodyBB = llvm::BasicBlock::Create(*context_, "lc.body", currentFunction_);
            auto* storeBB = llvm::BasicBlock::Create(*context_, "lc.store", currentFunction_);
            auto* skipBB  = llvm::BasicBlock::Create(*context_, "lc.skip", currentFunction_);
            auto* endBB   = llvm::BasicBlock::Create(*context_, "lc.end", currentFunction_);

            builder_->CreateBr(condBB);

            // Condition
            builder_->SetInsertPoint(condBB);
            auto* cur = builder_->CreateLoad(varType, loopVar, "lc.cur");
            llvm::Value* cond;
            if (isInclusive)
                cond = typeInfo->isSigned
                     ? builder_->CreateICmpSLE(cur, endVal, "lc.cmp")
                     : builder_->CreateICmpULE(cur, endVal, "lc.cmp");
            else
                cond = typeInfo->isSigned
                     ? builder_->CreateICmpSLT(cur, endVal, "lc.cmp")
                     : builder_->CreateICmpULT(cur, endVal, "lc.cmp");
            builder_->CreateCondBr(cond, bodyBB, endBB);

            // Body — evaluate filter
            builder_->SetInsertPoint(bodyBB);
            auto* val = std::any_cast<llvm::Value*>(visit(valExpr));
            auto* filterVal = std::any_cast<llvm::Value*>(visit(filterExpr));
            builder_->CreateCondBr(filterVal, storeBB, skipBB);

            // Store — filter passed
            builder_->SetInsertPoint(storeBB);
            auto* wi = builder_->CreateLoad(i64Ty, writeIdx, "lc.widx");
            auto* gep = builder_->CreateInBoundsGEP(
                arrTy, arrAlloca,
                { llvm::ConstantInt::get(i64Ty, 0), wi }, "lc.gep");
            builder_->CreateStore(val, gep);
            auto* wiInc = builder_->CreateAdd(wi,
                llvm::ConstantInt::get(i64Ty, 1), "lc.widx.inc");
            builder_->CreateStore(wiInc, writeIdx);
            builder_->CreateBr(skipBB);

            // Skip/Update
            builder_->SetInsertPoint(skipBB);
            auto* curVar = builder_->CreateLoad(varType, loopVar, "lc.var");
            auto* nextVar = builder_->CreateAdd(curVar,
                llvm::ConstantInt::get(varType, 1), "lc.var.inc");
            builder_->CreateStore(nextVar, loopVar);
            builder_->CreateBr(condBB);

            builder_->SetInsertPoint(endBB);
            locals_.erase(varName);

            auto* result = builder_->CreateLoad(arrTy, arrAlloca, "lc.result");
            return static_cast<llvm::Value*>(result);
        }
    }

    std::cerr << "lux: list comprehension only supports range iterables\n";
    return static_cast<llvm::Value*>(
        llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
}

std::any IRGen::visitIndexExpr(LuxParser::IndexExprContext* ctx) {
    // Collect all chained indices: arr[i][j][k] → varName + [i, j, k]
    std::vector<LuxParser::ExpressionContext*> indexExprs;
    std::string varName;

    auto* current = static_cast<LuxParser::ExpressionContext*>(ctx);
    while (auto* idxCtx = dynamic_cast<LuxParser::IndexExprContext*>(current)) {
        indexExprs.push_back(idxCtx->expression(1)); // the index
        current = idxCtx->expression(0);              // the base
    }

    auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(current);
    if (!identBase) {
        // ── Field-access base: e.g. cell.chars[0] ─────────────────────
        auto* fieldBase = dynamic_cast<LuxParser::FieldAccessExprContext*>(current);
        if (fieldBase) {
            auto fieldName = fieldBase->IDENTIFIER()->getText();
            auto* innerIdent = dynamic_cast<LuxParser::IdentExprContext*>(
                fieldBase->expression());
            if (innerIdent) {
                auto structVar = innerIdent->IDENTIFIER()->getText();
                auto lit = locals_.find(structVar);
                if (lit != locals_.end() && lit->second.typeInfo) {
                    auto* structTI = lit->second.typeInfo;
                    int fieldIdx = -1;
                    const TypeInfo* fieldTI = nullptr;
                    for (size_t f = 0; f < structTI->fields.size(); f++) {
                        if (structTI->fields[f].name == fieldName) {
                            fieldIdx = static_cast<int>(f);
                            fieldTI = structTI->fields[f].typeInfo;
                            break;
                        }
                    }
                    if (fieldIdx >= 0 && fieldTI && fieldTI->arraySize > 0) {
                        auto* structTy = lit->second.alloca->getAllocatedType();
                        auto* fieldGep = builder_->CreateStructGEP(
                            structTy, lit->second.alloca, fieldIdx,
                            structVar + "_" + fieldName + "_ptr");
                        auto* arrTy = fieldTI->toLLVMType(
                            *context_, module_->getDataLayout());
                        auto* i64Ty = llvm::Type::getInt64Ty(*context_);
                        auto* idx = std::any_cast<llvm::Value*>(
                            visit(indexExprs[0]));
                        if (idx->getType() != i64Ty)
                            idx = builder_->CreateIntCast(idx, i64Ty, true);
                        auto* elemGep = builder_->CreateInBoundsGEP(
                            arrTy, fieldGep,
                            { llvm::ConstantInt::get(i64Ty, 0), idx },
                            fieldName + "_elem");
                        auto* elemTy = llvm::cast<llvm::ArrayType>(arrTy)
                            ->getElementType();
                        return static_cast<llvm::Value*>(
                            builder_->CreateLoad(elemTy, elemGep,
                                fieldName + "_val"));
                    }
                }
            }
        }
        std::cerr << "lux: invalid index base expression\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    varName = identBase->IDENTIFIER()->getText();

    // Reverse since we collected from outermost to innermost
    std::reverse(indexExprs.begin(), indexExprs.end());

    // ── Variadic parameter index access ─────────────────────────────
    auto vit = variadicParams_.find(varName);
    if (vit != variadicParams_.end()) {
        auto* ptrTy = llvm::PointerType::getUnqual(*context_);
        auto* i64Ty = llvm::Type::getInt64Ty(*context_);
        auto* dataPtr = builder_->CreateLoad(ptrTy, vit->second.dataPtr, "var_data");
        auto* elemTy = vit->second.elementType->toLLVMType(*context_, module_->getDataLayout());
        auto* idx = std::any_cast<llvm::Value*>(visit(indexExprs[0]));
        if (idx->getType() != i64Ty)
            idx = builder_->CreateIntCast(idx, i64Ty, true);
        auto* gep = builder_->CreateGEP(elemTy, dataPtr, idx, varName + "_elem");
        return static_cast<llvm::Value*>(builder_->CreateLoad(elemTy, gep));
    }

    auto it = locals_.find(varName);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << varName << "'\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    // ── Extended type (Vec<T>, Map<K,V>) index access ──────────────────
    if (it->second.typeInfo && it->second.typeInfo->kind == TypeKind::Extended) {
        auto* ti = it->second.typeInfo;
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);

        if (ti->extendedKind == "Map") {
            // Map<K,V> subscript: m[key] → lux_map_get_<suffix>
            auto* keyLLTy = ti->keyType->toLLVMType(*context_, module_->getDataLayout());
            auto* valLLTy = ti->valueType->toLLVMType(*context_, module_->getDataLayout());
            auto suffix = ti->builtinSuffix;

            auto* keyVal = std::any_cast<llvm::Value*>(visit(indexExprs[0]));
            if (keyVal->getType() != keyLLTy) {
                if (keyVal->getType()->isIntegerTy() && keyLLTy->isIntegerTy())
                    keyVal = builder_->CreateIntCast(keyVal, keyLLTy, ti->keyType->isSigned);
            }

            auto callee = declareBuiltin(
                "lux_map_get_" + suffix, valLLTy, { ptrTy, keyLLTy });
            auto* result = builder_->CreateCall(callee,
                { it->second.alloca, keyVal }, "map_get");
            return static_cast<llvm::Value*>(result);
        }

        // Vec<T> subscript: v[i] → lux_vec_at_<suffix>
        auto* elemLLTy = ti->elementType->toLLVMType(
            *context_, module_->getDataLayout());
        auto suffix = getVecSuffix(ti->elementType);

        auto* idx = std::any_cast<llvm::Value*>(visit(indexExprs[0]));
        if (idx->getType() != usizeTy)
            idx = builder_->CreateIntCast(idx, usizeTy, false);

        auto callee = declareBuiltin(
            "lux_vec_at_" + suffix, elemLLTy, { ptrTy, usizeTy });
        auto* result = builder_->CreateCall(callee,
            { it->second.alloca, idx }, "vec_at");
        return static_cast<llvm::Value*>(result);
    }

    // ── Pointer index access (ptr[i]) ─────────────────────────────
    auto* alloca    = it->second.alloca;
    auto* allocType = alloca->getAllocatedType();
    auto* ti = it->second.typeInfo;
    if (ti && ti->kind == TypeKind::Pointer && ti->pointeeType) {
        auto* ptrTy  = llvm::PointerType::getUnqual(*context_);
        auto* elemTy = ti->pointeeType->toLLVMType(*context_, module_->getDataLayout());
        auto* i64Ty  = llvm::Type::getInt64Ty(*context_);

        // Load the pointer value from the alloca
        auto* ptrVal = builder_->CreateLoad(ptrTy, alloca, varName + "_ptr");

        // GEP with the first index
        auto* idx = std::any_cast<llvm::Value*>(visit(indexExprs[0]));
        if (idx->getType() != i64Ty)
            idx = builder_->CreateIntCast(idx, i64Ty, true);

        auto* gep = builder_->CreateGEP(elemTy, ptrVal, idx, varName + "_elem");
        return static_cast<llvm::Value*>(builder_->CreateLoad(elemTy, gep));
    }

    // ── Array index access (original path) ──────────────────────────
    auto* i64Ty     = llvm::Type::getInt64Ty(*context_);

    std::vector<llvm::Value*> gepIndices;
    gepIndices.push_back(llvm::ConstantInt::get(i64Ty, 0));
    for (auto* idxExpr : indexExprs) {
        auto* idx = std::any_cast<llvm::Value*>(visit(idxExpr));
        if (idx->getType() != i64Ty)
            idx = builder_->CreateIntCast(idx, i64Ty, true);
        gepIndices.push_back(idx);
    }

    auto* gep = builder_->CreateGEP(allocType, alloca, gepIndices, varName + "_elem");

    // Determine element type by unwrapping array types
    llvm::Type* elemTy = allocType;
    for (size_t i = 0; i < indexExprs.size(); i++)
        elemTy = llvm::cast<llvm::ArrayType>(elemTy)->getElementType();

    return static_cast<llvm::Value*>(builder_->CreateLoad(elemTy, gep));
}

std::any IRGen::visitStructLitExpr(LuxParser::StructLitExprContext* ctx) {
    auto identifiers = ctx->IDENTIFIER();
    auto typeName = identifiers[0]->getText();
    auto* ti = typeRegistry_.lookup(typeName);
    if (!ti || (ti->kind != TypeKind::Struct && ti->kind != TypeKind::Union)) {
        std::cerr << "lux: unknown struct/union type '" << typeName << "'\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    // ── Union literal: Name { field: value } (exactly one field) ────
    if (ti->kind == TypeKind::Union) {
        auto* unionTy = ti->toLLVMType(*context_, module_->getDataLayout());
        auto* alloca = builder_->CreateAlloca(unionTy, nullptr, typeName + "_tmp");

        auto exprs = ctx->expression();
        if (!exprs.empty()) {
            auto fieldName = identifiers[1]->getText();
            const TypeInfo* fieldTI = nullptr;
            for (auto& f : ti->fields) {
                if (f.name == fieldName) { fieldTI = f.typeInfo; break; }
            }
            if (fieldTI) {
                auto* val = std::any_cast<llvm::Value*>(visit(exprs[0]));
                auto* fieldLLTy = fieldTI->toLLVMType(*context_, module_->getDataLayout());
                if (val->getType() != fieldLLTy) {
                    if (val->getType()->isIntegerTy() && fieldLLTy->isIntegerTy())
                        val = builder_->CreateIntCast(val, fieldLLTy, fieldTI->isSigned);
                    else if (val->getType()->isFloatingPointTy() && fieldLLTy->isFloatingPointTy()) {
                        if (val->getType()->getPrimitiveSizeInBits() > fieldLLTy->getPrimitiveSizeInBits())
                            val = builder_->CreateFPTrunc(val, fieldLLTy);
                        else
                            val = builder_->CreateFPExt(val, fieldLLTy);
                    }
                }
                // Store: cast union pointer to field pointer type, then store
                builder_->CreateStore(val, alloca);
            }
        }

        return static_cast<llvm::Value*>(
            builder_->CreateLoad(unionTy, alloca, typeName + "_val"));
    }

    // ── Struct literal ──────────────────────────────────────────────
    auto* structTy = ti->toLLVMType(*context_, module_->getDataLayout());
    llvm::Value* agg = llvm::ConstantAggregateZero::get(structTy);

    // Parse field initializers: Name { field1: expr1, field2: expr2 }
    auto exprs = ctx->expression();
    for (size_t i = 0; i < exprs.size(); i++) {
        auto fieldName = identifiers[i + 1]->getText();

        // Find field index
        int fieldIdx = -1;
        for (size_t f = 0; f < ti->fields.size(); f++) {
            if (ti->fields[f].name == fieldName) {
                fieldIdx = static_cast<int>(f);
                break;
            }
        }
        if (fieldIdx < 0) {
            std::cerr << "lux: '" << typeName
                      << "' has no field '" << fieldName << "'\n";
            continue;
        }

        auto* val = std::any_cast<llvm::Value*>(visit(exprs[i]));
        auto* fieldTI = ti->fields[fieldIdx].typeInfo;
        auto* fieldTy = fieldTI->toLLVMType(*context_, module_->getDataLayout());

        // Cast if needed
        if (val->getType() != fieldTy) {
            if (val->getType()->isIntegerTy() && fieldTy->isIntegerTy())
                val = builder_->CreateIntCast(val, fieldTy, fieldTI->isSigned);
            else if (val->getType()->isFloatingPointTy() && fieldTy->isFloatingPointTy()) {
                if (val->getType()->getPrimitiveSizeInBits() > fieldTy->getPrimitiveSizeInBits())
                    val = builder_->CreateFPTrunc(val, fieldTy);
                else
                    val = builder_->CreateFPExt(val, fieldTy);
            }
        }

        agg = builder_->CreateInsertValue(agg, val, {static_cast<unsigned>(fieldIdx)});
    }

    return static_cast<llvm::Value*>(agg);
}

std::any IRGen::visitFieldAccessExpr(LuxParser::FieldAccessExprContext* ctx) {
    auto fieldName = ctx->IDENTIFIER()->getText();

    // The base expression should resolve to a variable with a struct type
    auto* baseExpr = ctx->expression();

    // ── Case 1: base is a bare variable (e.g. myStruct.field) ──────
    auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(baseExpr);
    if (identBase) {
        auto varName = identBase->IDENTIFIER()->getText();

        // ── Variadic parameter .len access ──────────────────────────
        auto vit = variadicParams_.find(varName);
        if (vit != variadicParams_.end()) {
            if (fieldName == "len") {
                auto* i64Ty = llvm::Type::getInt64Ty(*context_);
                return static_cast<llvm::Value*>(
                    builder_->CreateLoad(i64Ty, vit->second.lenAlloca, "var_len"));
            }
            std::cerr << "lux: variadic parameter '" << varName
                      << "' has no field '" << fieldName << "'\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        auto it = locals_.find(varName);
        if (it == locals_.end()) {
            // Check if it's a C struct macro (e.g. RED.r where RED is a Color macro)
            auto smIt = cStructMacros_.find(varName);
            if (smIt != cStructMacros_.end()) {
                const auto* sm = smIt->second;
                auto* structTy = llvm::StructType::getTypeByName(*context_, sm->structType);
                if (structTy) {
                    // Build the ConstantStruct
                    std::vector<llvm::Constant*> fields;
                    unsigned numElements = structTy->getNumElements();
                    for (unsigned i = 0; i < numElements && i < sm->fieldValues.size(); ++i) {
                        auto* fTy = structTy->getElementType(i);
                        fields.push_back(llvm::ConstantInt::get(
                            fTy, static_cast<uint64_t>(sm->fieldValues[i]), true));
                    }
                    for (unsigned i = static_cast<unsigned>(sm->fieldValues.size());
                         i < numElements; ++i) {
                        fields.push_back(llvm::Constant::getNullValue(
                            structTy->getElementType(i)));
                    }
                    auto* constStruct = llvm::ConstantStruct::get(structTy, fields);

                    // Look up the TypeInfo for the struct
                    auto* structTI = typeRegistry_.lookup(sm->structType);
                    if (structTI) {
                        const TypeInfo* fieldTI = nullptr;
                        int fieldIdx = -1;
                        for (size_t f = 0; f < structTI->fields.size(); f++) {
                            if (structTI->fields[f].name == fieldName) {
                                fieldIdx = static_cast<int>(f);
                                fieldTI = structTI->fields[f].typeInfo;
                                break;
                            }
                        }
                        if (fieldIdx >= 0) {
                            return static_cast<llvm::Value*>(
                                builder_->CreateExtractValue(constStruct,
                                    {static_cast<unsigned>(fieldIdx)}, fieldName));
                        }
                        std::cerr << "lux: '" << sm->structType
                                  << "' has no field '" << fieldName << "'\n";
                    }
                }
                return static_cast<llvm::Value*>(
                    llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
            }

            std::cerr << "lux: undefined variable '" << varName << "'\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        auto* alloca   = it->second.alloca;
        auto* structTI = it->second.typeInfo;

        if (structTI->kind != TypeKind::Struct && structTI->kind != TypeKind::Union) {
            std::cerr << "lux: '" << varName << "' is not a struct or union\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        // Find field
        const TypeInfo* fieldTI = nullptr;
        int fieldIdx = -1;
        for (size_t f = 0; f < structTI->fields.size(); f++) {
            if (structTI->fields[f].name == fieldName) {
                fieldIdx = static_cast<int>(f);
                fieldTI = structTI->fields[f].typeInfo;
                break;
            }
        }
        if (fieldIdx < 0) {
            std::cerr << "lux: '" << structTI->name
                      << "' has no field '" << fieldName << "'\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        auto* fieldLLTy = fieldTI->toLLVMType(*context_, module_->getDataLayout());

        if (structTI->kind == TypeKind::Union) {
            return static_cast<llvm::Value*>(
                builder_->CreateLoad(fieldLLTy, alloca, fieldName));
        }

        auto* structTy = alloca->getAllocatedType();
        auto* gep = builder_->CreateStructGEP(structTy, alloca, fieldIdx, fieldName + "_ptr");
        return static_cast<llvm::Value*>(builder_->CreateLoad(fieldLLTy, gep, fieldName));
    }

    // ── Case 2: base is an indexed array element (e.g. arr[i].field) ──
    auto* indexBase = dynamic_cast<LuxParser::IndexExprContext*>(baseExpr);
    if (indexBase) {
        // Walk down to find the root variable and collect indices
        std::vector<LuxParser::ExpressionContext*> indexExprs;
        auto* current = static_cast<LuxParser::ExpressionContext*>(indexBase);
        while (auto* idxCtx = dynamic_cast<LuxParser::IndexExprContext*>(current)) {
            indexExprs.push_back(idxCtx->expression(1));
            current = idxCtx->expression(0);
        }
        auto* rootIdent = dynamic_cast<LuxParser::IdentExprContext*>(current);
        if (!rootIdent) {
            std::cerr << "lux: invalid base for indexed field access\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }
        std::reverse(indexExprs.begin(), indexExprs.end());
        auto varName = rootIdent->IDENTIFIER()->getText();

        auto it = locals_.find(varName);
        if (it == locals_.end()) {
            std::cerr << "lux: undefined variable '" << varName << "'\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        auto* alloca   = it->second.alloca;
        auto* allocType = alloca->getAllocatedType();
        auto* structTI = it->second.typeInfo;
        auto* i64Ty    = llvm::Type::getInt64Ty(*context_);

        if (structTI->kind != TypeKind::Struct && structTI->kind != TypeKind::Union) {
            std::cerr << "lux: elements of '" << varName << "' are not structs\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        // Build GEP indices for array indexing: [0, idx0, idx1, ...]
        std::vector<llvm::Value*> gepIndices;
        gepIndices.push_back(llvm::ConstantInt::get(i64Ty, 0));
        for (auto* idxExpr : indexExprs) {
            auto* idx = std::any_cast<llvm::Value*>(visit(idxExpr));
            if (idx->getType() != i64Ty)
                idx = builder_->CreateIntCast(idx, i64Ty, true);
            gepIndices.push_back(idx);
        }

        // GEP to the array element (a struct)
        auto* elemPtr = builder_->CreateGEP(allocType, alloca, gepIndices,
                                            varName + "_elem_ptr");

        // Determine the struct LLVM type (unwrap array types)
        llvm::Type* elemTy = allocType;
        for (size_t i = 0; i < indexExprs.size(); i++)
            elemTy = llvm::cast<llvm::ArrayType>(elemTy)->getElementType();

        // Find the field in the struct
        const TypeInfo* fieldTI = nullptr;
        int fieldIdx = -1;
        for (size_t f = 0; f < structTI->fields.size(); f++) {
            if (structTI->fields[f].name == fieldName) {
                fieldIdx = static_cast<int>(f);
                fieldTI = structTI->fields[f].typeInfo;
                break;
            }
        }
        if (fieldIdx < 0) {
            std::cerr << "lux: '" << structTI->name
                      << "' has no field '" << fieldName << "'\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        auto* fieldLLTy = fieldTI->toLLVMType(*context_, module_->getDataLayout());

        if (structTI->kind == TypeKind::Union) {
            return static_cast<llvm::Value*>(
                builder_->CreateLoad(fieldLLTy, elemPtr, fieldName));
        }

        auto* fieldGep = builder_->CreateStructGEP(elemTy, elemPtr, fieldIdx,
                                                    fieldName + "_ptr");
        return static_cast<llvm::Value*>(
            builder_->CreateLoad(fieldLLTy, fieldGep, fieldName));
    }

    // ── Case 3: base is another field access (e.g. a.b.c — chained) ──
    auto* fieldBase = dynamic_cast<LuxParser::FieldAccessExprContext*>(baseExpr);
    if (fieldBase) {
        // Evaluate chained field access: get the struct value, then access this field
        // We need the address, not the value. Walk down to find the root and rebuild GEP chain.
        // For now, use the general fallback approach: alloca the value and GEP into it.
        auto* baseVal = std::any_cast<llvm::Value*>(visit(baseExpr));
        if (baseVal->getType()->isStructTy()) {
            // Find the field in the intermediate struct
            auto* baseTI = resolveExprTypeInfo(baseExpr);
            if (baseTI && (baseTI->kind == TypeKind::Struct || baseTI->kind == TypeKind::Union)) {
                const TypeInfo* fieldTI = nullptr;
                int fieldIdx = -1;
                for (size_t f = 0; f < baseTI->fields.size(); f++) {
                    if (baseTI->fields[f].name == fieldName) {
                        fieldIdx = static_cast<int>(f);
                        fieldTI = baseTI->fields[f].typeInfo;
                        break;
                    }
                }
                if (fieldIdx >= 0) {
                    auto* fieldLLTy = fieldTI->toLLVMType(*context_, module_->getDataLayout());
                    auto* tmp = builder_->CreateAlloca(baseVal->getType(), nullptr, "chain_tmp");
                    builder_->CreateStore(baseVal, tmp);
                    if (baseTI->kind == TypeKind::Union) {
                        return static_cast<llvm::Value*>(
                            builder_->CreateLoad(fieldLLTy, tmp, fieldName));
                    }
                    auto* gep = builder_->CreateStructGEP(baseVal->getType(), tmp,
                                                          fieldIdx, fieldName + "_ptr");
                    return static_cast<llvm::Value*>(
                        builder_->CreateLoad(fieldLLTy, gep, fieldName));
                }
            }
        }
    }

    std::cerr << "lux: unsupported base expression for field access\n";
    return static_cast<llvm::Value*>(
        llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
}

std::any IRGen::visitArrowAccessExpr(LuxParser::ArrowAccessExprContext* ctx) {
    auto fieldName = ctx->IDENTIFIER()->getText();

    // Evaluate base expression — should yield a pointer to struct
    auto* ptrVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    // Resolve the pointee struct TypeInfo from the expression tree
    auto* exprTI = resolveExprTypeInfo(ctx->expression());
    const TypeInfo* structTI = nullptr;
    if (exprTI && exprTI->kind == TypeKind::Pointer)
        structTI = exprTI->pointeeType;

    if (!structTI || (structTI->kind != TypeKind::Struct && structTI->kind != TypeKind::Union)) {
        std::cerr << "lux: '->' requires pointer to struct or union\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    // Find field
    const TypeInfo* fieldTI = nullptr;
    int fieldIdx = -1;
    for (size_t f = 0; f < structTI->fields.size(); f++) {
        if (structTI->fields[f].name == fieldName) {
            fieldIdx = static_cast<int>(f);
            fieldTI = structTI->fields[f].typeInfo;
            break;
        }
    }
    if (fieldIdx < 0) {
        std::cerr << "lux: '" << structTI->name
                  << "' has no field '" << fieldName << "'\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    auto* fieldLLTy = fieldTI->toLLVMType(*context_, module_->getDataLayout());

    if (structTI->kind == TypeKind::Union) {
        // Union: all fields at offset 0
        return static_cast<llvm::Value*>(
            builder_->CreateLoad(fieldLLTy, ptrVal, fieldName));
    }

    auto* structTy = structTI->toLLVMType(*context_, module_->getDataLayout());
    auto* gep = builder_->CreateStructGEP(structTy, ptrVal, fieldIdx, fieldName + "_ptr");

    return static_cast<llvm::Value*>(builder_->CreateLoad(fieldLLTy, gep, fieldName));
}

std::any IRGen::visitEnumAccessExpr(LuxParser::EnumAccessExprContext* ctx) {
    auto identifiers = ctx->IDENTIFIER();
    auto enumName = identifiers[0]->getText();
    auto variantName = identifiers[1]->getText();

    auto* ti = typeRegistry_.lookup(enumName);
    if (!ti || ti->kind != TypeKind::Enum) {
        std::cerr << "lux: unknown enum type '" << enumName << "'\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    int variantIdx = -1;
    for (size_t i = 0; i < ti->enumVariants.size(); i++) {
        if (ti->enumVariants[i] == variantName) {
            variantIdx = static_cast<int>(i);
            break;
        }
    }
    if (variantIdx < 0) {
        std::cerr << "lux: enum '" << enumName
                  << "' has no variant '" << variantName << "'\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    return static_cast<llvm::Value*>(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), variantIdx));
}

// ── Static method call: Struct::method(args) ────────────────────────────────
std::any IRGen::visitStaticMethodCallExpr(
        LuxParser::StaticMethodCallExprContext* ctx) {
    auto ids = ctx->IDENTIFIER();
    auto structName = ids[0]->getText();
    auto methodName = ids[1]->getText();

    auto smIt = staticStructMethods_.find(structName);
    if (smIt == staticStructMethods_.end()) {
        std::cerr << "lux: struct '" << structName << "' has no static methods\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto mIt = smIt->second.find(methodName);
    if (mIt == smIt->second.end()) {
        std::cerr << "lux: struct '" << structName
                  << "' has no static method '" << methodName << "'\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    auto* fn = mIt->second;

    // Collect arguments
    std::vector<llvm::Value*> callArgs;
    if (auto* argList = ctx->argList()) {
        auto* fnType = fn->getFunctionType();
        size_t paramIdx = 0;
        for (auto* argExpr : argList->expression()) {
            auto* argVal = std::any_cast<llvm::Value*>(visit(argExpr));
            if (paramIdx < fnType->getNumParams()) {
                auto* paramTy = fnType->getParamType(paramIdx);
                if (argVal->getType() != paramTy) {
                    if (argVal->getType()->isIntegerTy() && paramTy->isIntegerTy())
                        argVal = builder_->CreateIntCast(argVal, paramTy, true);
                    else if (argVal->getType()->isFloatingPointTy() && paramTy->isFloatingPointTy())
                        argVal = builder_->CreateFPCast(argVal, paramTy);
                }
            }
            callArgs.push_back(argVal);
            paramIdx++;
        }
    }

    auto* retTy = fn->getReturnType();
    if (retTy->isVoidTy()) {
        builder_->CreateCall(fn, callArgs);
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    return static_cast<llvm::Value*>(
        builder_->CreateCall(fn, callArgs, "static_call"));
}

std::any IRGen::visitTypeSpec(LuxParser::TypeSpecContext* ctx) {
    auto* ti = resolveTypeInfo(ctx);
    return static_cast<llvm::Type*>(ti->toLLVMType(*context_, module_->getDataLayout()));
}

std::any IRGen::visitNullLitExpr(LuxParser::NullLitExprContext* /*ctx*/) {
    return static_cast<llvm::Value*>(
        llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(*context_)));
}

std::any IRGen::visitAddrOfExpr(LuxParser::AddrOfExprContext* ctx) {
    auto varName = ctx->IDENTIFIER()->getText();
    auto it = locals_.find(varName);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << varName << "'\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::PointerType::getUnqual(*context_)));
    }
    // The alloca IS the pointer to the variable
    return static_cast<llvm::Value*>(it->second.alloca);
}

std::any IRGen::visitDerefExpr(LuxParser::DerefExprContext* ctx) {
    auto* ptrVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    // Find pointee type: first try AST-level resolution, then fall back to IR
    const TypeInfo* pointeeTI = nullptr;
    auto* exprTI = resolveExprTypeInfo(ctx->expression());
    if (exprTI && exprTI->kind == TypeKind::Pointer) {
        pointeeTI = exprTI->pointeeType;
    }

    // Fallback: inspect the LLVM IR value
    if (!pointeeTI) {
        if (auto* load = llvm::dyn_cast<llvm::LoadInst>(ptrVal)) {
            auto* src = load->getPointerOperand();
            for (auto& [vname, info] : locals_) {
                if (info.alloca == src && info.typeInfo->kind == TypeKind::Pointer) {
                    pointeeTI = info.typeInfo->pointeeType;
                    break;
                }
            }
        }
    }

    if (!pointeeTI) {
        std::cerr << "lux: cannot dereference non-pointer expression\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    auto* pointeeLLVM = pointeeTI->toLLVMType(*context_, module_->getDataLayout());
    return static_cast<llvm::Value*>(
        builder_->CreateLoad(pointeeLLVM, ptrVal, "deref"));
}

std::any IRGen::visitDerefAssignStmt(LuxParser::DerefAssignStmtContext* ctx) {
    llvm::Value* ptrVal = nullptr;
    const TypeInfo* ptrTI = nullptr;
    LuxParser::ExpressionContext* rhsExpr = nullptr;

    if (ctx->IDENTIFIER()) {
        // *ptr = value;
        auto varName = ctx->IDENTIFIER()->getText();
        auto it = locals_.find(varName);
        if (it == locals_.end()) {
            std::cerr << "lux: undefined variable '" << varName << "'\n";
            return {};
        }

        auto* alloca = it->second.alloca;
        ptrTI = it->second.typeInfo;

        if (ptrTI->kind != TypeKind::Pointer) {
            std::cerr << "lux: '" << varName << "' is not a pointer\n";
            return {};
        }

        ptrVal = builder_->CreateLoad(
            llvm::PointerType::getUnqual(*context_), alloca, varName + "_load");
        rhsExpr = ctx->expression(0);
    } else {
        // *(expr) = value;
        ptrVal = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));

        // Resolve pointer TypeInfo from the expression
        auto* exprTI = resolveExprTypeInfo(ctx->expression(0));
        if (exprTI && exprTI->kind == TypeKind::Pointer) {
            ptrTI = exprTI;
        }

        rhsExpr = ctx->expression(1);
    }

    // Evaluate RHS
    auto* val = std::any_cast<llvm::Value*>(visit(rhsExpr));

    // Cast if needed
    if (ptrTI && ptrTI->pointeeType) {
        auto* pointeeTy = ptrTI->pointeeType->toLLVMType(*context_, module_->getDataLayout());
        if (val->getType() != pointeeTy) {
            if (val->getType()->isIntegerTy() && pointeeTy->isIntegerTy())
                val = builder_->CreateIntCast(val, pointeeTy, ptrTI->pointeeType->isSigned);
            else if (val->getType()->isFloatingPointTy() && pointeeTy->isFloatingPointTy()) {
                if (val->getType()->getPrimitiveSizeInBits() > pointeeTy->getPrimitiveSizeInBits())
                    val = builder_->CreateFPTrunc(val, pointeeTy);
                else
                    val = builder_->CreateFPExt(val, pointeeTy);
            }
        }
    }

    builder_->CreateStore(val, ptrVal);
    return {};
}

std::any IRGen::visitFnCallExpr(LuxParser::FnCallExprContext* ctx) {
    auto* baseExpr = ctx->expression();

    // Resolve the function TypeInfo from the base expression
    const TypeInfo* fnTI = nullptr;
    llvm::Function* directFn = nullptr;
    std::string calleeName;

    // Check if base is a simple identifier
    if (auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(baseExpr)) {
        calleeName = identBase->IDENTIFIER()->getText();

        // ── Global builtins (value-returning: toInt, toFloat, toBool, toString) ──
        if (globalBuiltins_.count(calleeName)) {
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
            auto* strTy   = typeRegistry_.lookup("string")
                                ->toLLVMType(*context_, module_->getDataLayout());

            // Collect arguments
            std::vector<llvm::Value*> args;
            if (auto* argList = ctx->argList()) {
                for (auto* exprCtx : argList->expression())
                    args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
            }

            if (calleeName == "toInt") {
                auto* strPtr = builder_->CreateExtractValue(args[0], 0, "toInt_ptr");
                auto* strLen = builder_->CreateExtractValue(args[0], 1, "toInt_len");
                auto callee = declareBuiltin("lux_toInt",
                    llvm::Type::getInt64Ty(*context_), {ptrTy, usizeTy});
                auto* result = builder_->CreateCall(callee, {strPtr, strLen}, "toInt");
                return static_cast<llvm::Value*>(result);
            }
            if (calleeName == "toFloat") {
                auto* strPtr = builder_->CreateExtractValue(args[0], 0, "toFloat_ptr");
                auto* strLen = builder_->CreateExtractValue(args[0], 1, "toFloat_len");
                auto callee = declareBuiltin("lux_toFloat",
                    llvm::Type::getDoubleTy(*context_), {ptrTy, usizeTy});
                auto* result = builder_->CreateCall(callee, {strPtr, strLen}, "toFloat");
                return static_cast<llvm::Value*>(result);
            }
            if (calleeName == "toBool") {
                auto* strPtr = builder_->CreateExtractValue(args[0], 0, "toBool_ptr");
                auto* strLen = builder_->CreateExtractValue(args[0], 1, "toBool_len");
                auto callee = declareBuiltin("lux_toBool",
                    llvm::Type::getInt1Ty(*context_), {ptrTy, usizeTy});
                auto* result = builder_->CreateCall(callee, {strPtr, strLen}, "toBool");
                return static_cast<llvm::Value*>(result);
            }
            if (calleeName == "toString") {
                auto* arg     = args[0];
                auto* argType = arg->getType();

                // Detect type info via variable lookup
                const TypeInfo* argTI = nullptr;
                auto argExprs = ctx->argList()->expression();
                if (auto* argIdent = dynamic_cast<LuxParser::IdentExprContext*>(argExprs[0])) {
                    auto varIt = locals_.find(argIdent->IDENTIFIER()->getText());
                    if (varIt != locals_.end())
                        argTI = varIt->second.typeInfo;
                }

                // Detect char literal directly from the AST
                bool isCharLit = dynamic_cast<LuxParser::CharLitExprContext*>(argExprs[0]) != nullptr;

                std::string suffix;
                if (argTI) {
                    suffix = argTI->builtinSuffix;
                } else if (isCharLit)                 { suffix = "char"; }
                else if (argType->isIntegerTy(1))     { suffix = "bool"; }
                else if (argType->isIntegerTy(8))     { suffix = "i8"; }
                else if (argType->isIntegerTy(16))    { suffix = "i16"; }
                else if (argType->isIntegerTy(32))    { suffix = "i32"; }
                else if (argType->isIntegerTy(64))    { suffix = "i64"; }
                else if (argType->isFloatTy())        { suffix = "f32"; }
                else if (argType->isDoubleTy())       { suffix = "f64"; }
                else                                  { suffix = "i32"; }

                auto cFuncName = "lux_toString_" + suffix;

                // toString returns a struct {ptr, i64} matching the string ABI
                auto* retStructTy = llvm::StructType::get(*context_,
                    {ptrTy, usizeTy});
                auto callee = declareBuiltin(cFuncName, retStructTy, {argType});
                auto* retVal = builder_->CreateCall(callee, {arg}, "toStr");

                // Extract ptr and len, build lux string fat pointer
                auto* retPtr = builder_->CreateExtractValue(retVal, 0, "toStr_ptr");
                auto* retLen = builder_->CreateExtractValue(retVal, 1, "toStr_len");
                llvm::Value* strStruct = llvm::UndefValue::get(strTy);
                strStruct = builder_->CreateInsertValue(strStruct, retPtr, 0, "str_ptr");
                strStruct = builder_->CreateInsertValue(strStruct, retLen, 1, "str_len");
                return static_cast<llvm::Value*>(strStruct);
            }

            // ── cstr(string) -> *char ──
            if (calleeName == "cstr") {
                auto* strPtr = builder_->CreateExtractValue(args[0], 0, "cstr_ptr");
                auto* strLen = builder_->CreateExtractValue(args[0], 1, "cstr_len");
                auto callee = declareBuiltin("lux_cstr", ptrTy, {ptrTy, usizeTy});
                auto* result = builder_->CreateCall(callee, {strPtr, strLen}, "cstr");
                return static_cast<llvm::Value*>(result);
            }

            // ── fromCStr(*char) -> string ──
            if (calleeName == "fromCStr") {
                auto* retStructTy = llvm::StructType::get(*context_, {ptrTy, usizeTy});
                auto callee = declareBuiltin("lux_fromCStr", retStructTy, {ptrTy});
                auto* retVal = builder_->CreateCall(callee, {args[0]}, "fromCStr");
                auto* retPtr = builder_->CreateExtractValue(retVal, 0, "fromCStr_ptr");
                auto* retLen = builder_->CreateExtractValue(retVal, 1, "fromCStr_len");
                llvm::Value* strStruct = llvm::UndefValue::get(strTy);
                strStruct = builder_->CreateInsertValue(strStruct, retPtr, 0, "str_ptr");
                strStruct = builder_->CreateInsertValue(strStruct, retLen, 1, "str_len");
                return static_cast<llvm::Value*>(strStruct);
            }

            // ── fromCStrLen(*char, usize) -> string ──
            if (calleeName == "fromCStrLen") {
                auto* retStructTy = llvm::StructType::get(*context_, {ptrTy, usizeTy});
                auto* lenArg = args[1];
                if (lenArg->getType() != usizeTy)
                    lenArg = builder_->CreateIntCast(lenArg, usizeTy, false, "fromCStrLen_cast");
                auto callee = declareBuiltin("lux_fromCStrLen", retStructTy, {ptrTy, usizeTy});
                auto* retVal = builder_->CreateCall(callee, {args[0], lenArg}, "fromCStrLen");
                auto* retPtr = builder_->CreateExtractValue(retVal, 0, "fromCStrLen_ptr");
                auto* retLen = builder_->CreateExtractValue(retVal, 1, "fromCStrLen_len");
                llvm::Value* strStruct = llvm::UndefValue::get(strTy);
                strStruct = builder_->CreateInsertValue(strStruct, retPtr, 0, "str_ptr");
                strStruct = builder_->CreateInsertValue(strStruct, retLen, 1, "str_len");
                return static_cast<llvm::Value*>(strStruct);
            }
        }

        // ── sprintf() as expression: format string with {} placeholders ──
        if (calleeName == "sprintf" && imports_.isImported("sprintf")) {
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
            auto* strTy   = typeRegistry_.lookup("string")
                                ->toLLVMType(*context_, module_->getDataLayout());

            std::vector<llvm::Value*> args;
            if (auto* argList = ctx->argList()) {
                for (auto* exprCtx : argList->expression())
                    args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
            }

            // First arg is the format string
            auto* fmtPtr = builder_->CreateExtractValue(args[0], 0, "fmt_ptr");
            auto* fmtLen = builder_->CreateExtractValue(args[0], 1, "fmt_len");

            // Build StringSlice struct type: { ptr, i64 }
            auto* sliceTy = llvm::StructType::get(*context_, {ptrTy, usizeTy});
            auto* i64Ty = llvm::Type::getInt64Ty(*context_);

            llvm::Value* argsPtr;
            llvm::Value* argsCount;

            if (args.size() <= 1) {
                // No format args
                argsPtr   = llvm::ConstantPointerNull::get(ptrTy);
                argsCount = llvm::ConstantInt::get(usizeTy, 0);
            } else {
                // Pack remaining string args into array of {ptr, len} structs
                size_t numArgs = args.size() - 1;
                auto* arrTy = llvm::ArrayType::get(sliceTy, numArgs);
                auto* arrAlloca = builder_->CreateAlloca(arrTy, nullptr, "sprintf.args");
                auto* zero = llvm::ConstantInt::get(i64Ty, 0);

                for (size_t i = 0; i < numArgs; i++) {
                    auto* idx = llvm::ConstantInt::get(i64Ty, i);
                    auto* gep = builder_->CreateGEP(arrTy, arrAlloca, {zero, idx});
                    builder_->CreateStore(args[i + 1], gep);
                }

                argsPtr   = builder_->CreateGEP(arrTy, arrAlloca,
                                {zero, llvm::ConstantInt::get(i64Ty, 0)}, "args_ptr");
                argsCount = llvm::ConstantInt::get(usizeTy, numArgs);
            }

            auto callee = declareBuiltin("lux_sprintf", strTy,
                {ptrTy, usizeTy, ptrTy, usizeTy});
            auto* ret = builder_->CreateCall(callee,
                {fmtPtr, fmtLen, argsPtr, argsCount}, "sprintf");

            auto* retPtr = builder_->CreateExtractValue(ret, 0, "sprintf_ptr");
            auto* retLen = builder_->CreateExtractValue(ret, 1, "sprintf_len");
            llvm::Value* strStruct = llvm::UndefValue::get(strTy);
            strStruct = builder_->CreateInsertValue(strStruct, retPtr, 0);
            strStruct = builder_->CreateInsertValue(strStruct, retLen, 1);
            return static_cast<llvm::Value*>(strStruct);
        }

        // ── dbg() as expression: print debug info and return value ──
        if (calleeName == "dbg" && imports_.isImported("dbg")) {
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
            auto* i32Ty   = llvm::Type::getInt32Ty(*context_);

            std::vector<llvm::Value*> args;
            if (auto* argList = ctx->argList()) {
                for (auto* exprCtx : argList->expression())
                    args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
            }

            if (args.empty()) {
                std::cerr << "lux: dbg() requires an argument\n";
                return static_cast<llvm::Value*>(
                    llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
            }

            auto* arg     = args[0];
            auto* argType = arg->getType();

            auto fileName = module_->getName().str();
            auto* fileGlobal = builder_->CreateGlobalString(fileName, ".dbg_file", 0, module_);
            auto* fileLen = llvm::ConstantInt::get(usizeTy, fileName.size());
            auto* lineNo  = llvm::ConstantInt::get(i32Ty, ctx->getStart()->getLine());

            const TypeInfo* argTI = nullptr;
            auto argExprs = ctx->argList()->expression();
            if (auto* argIdent = dynamic_cast<LuxParser::IdentExprContext*>(argExprs[0])) {
                auto varIt = locals_.find(argIdent->IDENTIFIER()->getText());
                if (varIt != locals_.end())
                    argTI = varIt->second.typeInfo;
            }

            bool isString = (argTI && argTI->kind == TypeKind::String)
                         || (!argTI && argType->isStructTy());

            if (isString) {
                auto* strPtr = builder_->CreateExtractValue(arg, 0, "dbg_str_ptr");
                auto* strLen = builder_->CreateExtractValue(arg, 1, "dbg_str_len");
                auto callee = declareBuiltin("lux_dbg_str",
                    llvm::Type::getVoidTy(*context_),
                    {ptrTy, usizeTy, i32Ty, ptrTy, usizeTy});
                builder_->CreateCall(callee, {fileGlobal, fileLen, lineNo, strPtr, strLen});
                return static_cast<llvm::Value*>(arg);
            }

            std::string suffix;
            if (argTI)                            suffix = argTI->builtinSuffix;
            else if (argType->isIntegerTy(1))     suffix = "bool";
            else if (argType->isIntegerTy(8))     suffix = "i8";
            else if (argType->isIntegerTy(16))    suffix = "i16";
            else if (argType->isIntegerTy(32))    suffix = "i32";
            else if (argType->isIntegerTy(64))    suffix = "i64";
            else if (argType->isIntegerTy(128))   suffix = "i128";
            else if (argType->isFloatTy())        suffix = "f32";
            else if (argType->isDoubleTy())       suffix = "f64";
            else                                  suffix = "i32";

            llvm::Value* callArg = arg;
            llvm::Type*  paramType = argType;
            if (argType->isIntegerTy(256)) {
                paramType = llvm::Type::getInt128Ty(*context_);
                callArg   = builder_->CreateTrunc(arg, paramType);
            }

            auto cFuncName = "lux_dbg_" + suffix;
            auto callee = declareBuiltin(cFuncName,
                llvm::Type::getVoidTy(*context_),
                {ptrTy, usizeTy, i32Ty, paramType});
            builder_->CreateCall(callee, {fileGlobal, fileLen, lineNo, callArg});
            return static_cast<llvm::Value*>(arg);
        }

        // ── std::io functions as expressions ────────────────────────────
        if (imports_.isImported(calleeName)) {
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
            auto* voidTy  = llvm::Type::getVoidTy(*context_);
            auto* i1Ty    = llvm::Type::getInt1Ty(*context_);
            auto* i8Ty    = llvm::Type::getInt8Ty(*context_);
            auto* i32Ty   = llvm::Type::getInt32Ty(*context_);
            auto* i64Ty   = llvm::Type::getInt64Ty(*context_);
            auto* f64Ty   = llvm::Type::getDoubleTy(*context_);
            auto* strTy   = llvm::StructType::get(*context_, {ptrTy, usizeTy});

            // Helper lambda: build string fat pointer from C return struct
            auto buildString = [&](llvm::Value* retVal) -> llvm::Value* {
                auto* retPtr = builder_->CreateExtractValue(retVal, 0, "io_str_ptr");
                auto* retLen = builder_->CreateExtractValue(retVal, 1, "io_str_len");
                llvm::Value* str = llvm::UndefValue::get(strTy);
                str = builder_->CreateInsertValue(str, retPtr, 0);
                str = builder_->CreateInsertValue(str, retLen, 1);
                return str;
            };

            // Helper lambda: extract string arg (ptr, len) from fat pointer
            auto extractStringArg = [&](llvm::Value* arg,
                                        std::vector<llvm::Value*>& out) {
                out.push_back(builder_->CreateExtractValue(arg, 0, "io_msg_ptr"));
                out.push_back(builder_->CreateExtractValue(arg, 1, "io_msg_len"));
            };

            // ── No-arg, returns string ──
            if (calleeName == "readLine" || calleeName == "readAll" ||
                calleeName == "readPassword") {
                auto callee = declareBuiltin("lux_" + calleeName, strTy, {});
                auto* ret = builder_->CreateCall(callee, {}, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── No-arg, returns primitive ──
            if (calleeName == "readChar") {
                auto callee = declareBuiltin("lux_readChar", i8Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "readChar");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "readInt") {
                auto callee = declareBuiltin("lux_readInt", i64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "readInt");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "readFloat") {
                auto callee = declareBuiltin("lux_readFloat", f64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "readFloat");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "readBool") {
                auto callee = declareBuiltin("lux_readBool", i32Ty, {});
                auto* ret32 = builder_->CreateCall(callee, {}, "readBool_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "readBool");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "readByte") {
                auto callee = declareBuiltin("lux_readByte", i8Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "readByte");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "isEOF") {
                auto callee = declareBuiltin("lux_isEOF", i32Ty, {});
                auto* ret32 = builder_->CreateCall(callee, {}, "isEOF_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "isEOF");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "isTTY") {
                auto callee = declareBuiltin("lux_isTTY", i32Ty, {});
                auto* ret32 = builder_->CreateCall(callee, {}, "isTTY_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "isTTY");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "isStdoutTTY") {
                auto callee = declareBuiltin("lux_isStdoutTTY", i32Ty, {});
                auto* ret32 = builder_->CreateCall(callee, {}, "isStdoutTTY_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "isStdoutTTY");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "isStderrTTY") {
                auto callee = declareBuiltin("lux_isStderrTTY", i32Ty, {});
                auto* ret32 = builder_->CreateCall(callee, {}, "isStderrTTY_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "isStderrTTY");
                return static_cast<llvm::Value*>(ret);
            }

            // ── String arg, returns string ──
            if (calleeName == "prompt" || calleeName == "promptPassword") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_" + calleeName, strTy,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── String arg, returns int64 ──
            if (calleeName == "promptInt") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_promptInt", i64Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "promptInt");
                return static_cast<llvm::Value*>(ret);
            }

            // ── String arg, returns float64 ──
            if (calleeName == "promptFloat") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_promptFloat", f64Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "promptFloat");
                return static_cast<llvm::Value*>(ret);
            }

            // ── String arg, returns bool ──
            if (calleeName == "promptBool") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_promptBool", i32Ty,
                    {ptrTy, usizeTy});
                auto* ret32 = builder_->CreateCall(callee, callArgs, "promptBool_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "promptBool");
                return static_cast<llvm::Value*>(ret);
            }

            // ── void functions as expression (flush, flushErr) — return undef ──
            if (calleeName == "flush") {
                auto callee = declareBuiltin("lux_flush", voidTy, {});
                builder_->CreateCall(callee, {});
                return static_cast<llvm::Value*>(
                    llvm::UndefValue::get(i32Ty));
            }
            if (calleeName == "flushErr") {
                auto callee = declareBuiltin("lux_flushErr", voidTy, {});
                builder_->CreateCall(callee, {});
                return static_cast<llvm::Value*>(
                    llvm::UndefValue::get(i32Ty));
            }

            // ── std::math — single-arg float64 → float64 ───────────────────
            static const std::unordered_set<std::string> mathF64Unary = {
                "sqrt", "cbrt", "exp", "exp2", "ln", "log2", "log10",
                "sin", "cos", "tan", "asin", "acos", "atan",
                "sinh", "cosh", "tanh",
                "ceil", "floor", "round", "trunc",
                "toRadians", "toDegrees"
            };
            if (mathF64Unary.count(calleeName)) {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* arg = args[0];
                if (arg->getType()->isIntegerTy())
                    arg = builder_->CreateSIToFP(arg, f64Ty, "math_cast");
                else if (arg->getType()->isFloatTy())
                    arg = builder_->CreateFPExt(arg, f64Ty, "math_ext");
                auto callee = declareBuiltin("lux_" + calleeName, f64Ty, {f64Ty});
                auto* ret = builder_->CreateCall(callee, {arg}, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::math — two-arg float64 → float64 ──────────────────────
            static const std::unordered_set<std::string> mathF64Binary = {
                "pow", "hypot", "atan2"
            };
            if (mathF64Binary.count(calleeName)) {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                for (auto*& a : args) {
                    if (a->getType()->isIntegerTy())
                        a = builder_->CreateSIToFP(a, f64Ty, "math_cast");
                    else if (a->getType()->isFloatTy())
                        a = builder_->CreateFPExt(a, f64Ty, "math_ext");
                }
                auto callee = declareBuiltin("lux_" + calleeName, f64Ty,
                    {f64Ty, f64Ty});
                auto* ret = builder_->CreateCall(callee, args, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::math — lerp(a, b, t) → float64 ────────────────────────
            if (calleeName == "lerp") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                for (auto*& a : args) {
                    if (a->getType()->isIntegerTy())
                        a = builder_->CreateSIToFP(a, f64Ty, "math_cast");
                    else if (a->getType()->isFloatTy())
                        a = builder_->CreateFPExt(a, f64Ty, "math_ext");
                }
                auto callee = declareBuiltin("lux_lerp", f64Ty,
                    {f64Ty, f64Ty, f64Ty});
                auto* ret = builder_->CreateCall(callee, args, "lerp");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::math — map(val, inMin, inMax, outMin, outMax) → float64
            if (calleeName == "map") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                for (auto*& a : args) {
                    if (a->getType()->isIntegerTy())
                        a = builder_->CreateSIToFP(a, f64Ty, "math_cast");
                    else if (a->getType()->isFloatTy())
                        a = builder_->CreateFPExt(a, f64Ty, "math_ext");
                }
                auto callee = declareBuiltin("lux_map", f64Ty,
                    {f64Ty, f64Ty, f64Ty, f64Ty, f64Ty});
                auto* ret = builder_->CreateCall(callee, args, "map");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::math — checks: float64 → bool ─────────────────────────
            static const std::unordered_set<std::string> mathChecks = {
                "isNaN", "isInf", "isFinite"
            };
            if (mathChecks.count(calleeName)) {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* arg = args[0];
                if (arg->getType()->isIntegerTy())
                    arg = builder_->CreateSIToFP(arg, f64Ty, "math_cast");
                else if (arg->getType()->isFloatTy())
                    arg = builder_->CreateFPExt(arg, f64Ty, "math_ext");
                auto callee = declareBuiltin("lux_" + calleeName, i32Ty,
                    {f64Ty});
                auto* ret32 = builder_->CreateCall(callee, {arg},
                    calleeName + "_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::math — polymorphic: abs(T) → T ────────────────────────
            if (calleeName == "abs") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* arg = args[0];
                auto* argType = arg->getType();

                // Detect type info via variable lookup
                const TypeInfo* argTI = nullptr;
                auto argExprs = ctx->argList()->expression();
                if (auto* argIdent = dynamic_cast<LuxParser::IdentExprContext*>(
                        argExprs[0])) {
                    auto varIt = locals_.find(argIdent->IDENTIFIER()->getText());
                    if (varIt != locals_.end())
                        argTI = varIt->second.typeInfo;
                }

                std::string suffix;
                if (argTI)                            suffix = argTI->builtinSuffix;
                else if (argType->isFloatTy())        suffix = "f32";
                else if (argType->isDoubleTy())       suffix = "f64";
                else if (argType->isIntegerTy(32))    suffix = "i32";
                else if (argType->isIntegerTy(64))    suffix = "i64";
                else { suffix = "i64"; arg = builder_->CreateIntCast(arg, i64Ty, true); argType = i64Ty; }

                // Cast to matching type for the C function
                llvm::Type* paramType = argType;
                if (suffix == "i32") paramType = i32Ty;
                else if (suffix == "i64") paramType = i64Ty;
                else if (suffix == "f32") paramType = llvm::Type::getFloatTy(*context_);
                else if (suffix == "f64") paramType = f64Ty;

                if (arg->getType() != paramType) {
                    if (paramType->isIntegerTy())
                        arg = builder_->CreateIntCast(arg, paramType, true);
                    else if (paramType->isFloatingPointTy() && arg->getType()->isIntegerTy())
                        arg = builder_->CreateSIToFP(arg, paramType);
                }

                auto callee = declareBuiltin("lux_abs_" + suffix,
                    paramType, {paramType});
                auto* ret = builder_->CreateCall(callee, {arg}, "abs");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::math — polymorphic: min/max(T, T) → T ─────────────────
            if (calleeName == "min" || calleeName == "max") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* a = args[0];
                auto* b = args[1];
                auto* argType = a->getType();

                const TypeInfo* argTI = nullptr;
                auto argExprs = ctx->argList()->expression();
                if (auto* argIdent = dynamic_cast<LuxParser::IdentExprContext*>(
                        argExprs[0])) {
                    auto varIt = locals_.find(argIdent->IDENTIFIER()->getText());
                    if (varIt != locals_.end())
                        argTI = varIt->second.typeInfo;
                }

                std::string suffix;
                if (argTI)                            suffix = argTI->builtinSuffix;
                else if (argType->isFloatTy())        suffix = "f32";
                else if (argType->isDoubleTy())       suffix = "f64";
                else if (argType->isIntegerTy(32))    suffix = "i32";
                else if (argType->isIntegerTy(64))    suffix = "i64";
                else { suffix = "i64"; a = builder_->CreateIntCast(a, i64Ty, true); b = builder_->CreateIntCast(b, i64Ty, true); argType = i64Ty; }

                llvm::Type* paramType = argType;
                if (suffix == "i32") paramType = i32Ty;
                else if (suffix == "i64") paramType = i64Ty;
                else if (suffix == "u32") paramType = i32Ty;
                else if (suffix == "u64") paramType = i64Ty;
                else if (suffix == "f32") paramType = llvm::Type::getFloatTy(*context_);
                else if (suffix == "f64") paramType = f64Ty;

                if (a->getType() != paramType)
                    a = builder_->CreateIntCast(a, paramType, suffix[0] != 'u');
                if (b->getType() != paramType)
                    b = builder_->CreateIntCast(b, paramType, suffix[0] != 'u');

                auto callee = declareBuiltin("lux_" + calleeName + "_" + suffix,
                    paramType, {paramType, paramType});
                auto* ret = builder_->CreateCall(callee, {a, b}, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::math — polymorphic: clamp(T, T, T) → T ────────────────
            if (calleeName == "clamp") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* val = args[0];
                auto* lo  = args[1];
                auto* hi  = args[2];
                auto* argType = val->getType();

                const TypeInfo* argTI = nullptr;
                auto argExprs = ctx->argList()->expression();
                if (auto* argIdent = dynamic_cast<LuxParser::IdentExprContext*>(
                        argExprs[0])) {
                    auto varIt = locals_.find(argIdent->IDENTIFIER()->getText());
                    if (varIt != locals_.end())
                        argTI = varIt->second.typeInfo;
                }

                std::string suffix;
                if (argTI)                            suffix = argTI->builtinSuffix;
                else if (argType->isFloatTy())        suffix = "f32";
                else if (argType->isDoubleTy())       suffix = "f64";
                else if (argType->isIntegerTy(32))    suffix = "i32";
                else if (argType->isIntegerTy(64))    suffix = "i64";
                else { suffix = "i64"; val = builder_->CreateIntCast(val, i64Ty, true); lo = builder_->CreateIntCast(lo, i64Ty, true); hi = builder_->CreateIntCast(hi, i64Ty, true); argType = i64Ty; }

                llvm::Type* paramType = argType;
                if (suffix == "i32") paramType = i32Ty;
                else if (suffix == "i64") paramType = i64Ty;
                else if (suffix == "u32") paramType = i32Ty;
                else if (suffix == "u64") paramType = i64Ty;
                else if (suffix == "f32") paramType = llvm::Type::getFloatTy(*context_);
                else if (suffix == "f64") paramType = f64Ty;

                if (val->getType() != paramType)
                    val = builder_->CreateIntCast(val, paramType, suffix[0] != 'u');
                if (lo->getType() != paramType)
                    lo = builder_->CreateIntCast(lo, paramType, suffix[0] != 'u');
                if (hi->getType() != paramType)
                    hi = builder_->CreateIntCast(hi, paramType, suffix[0] != 'u');

                auto callee = declareBuiltin("lux_clamp_" + suffix,
                    paramType, {paramType, paramType, paramType});
                auto* ret = builder_->CreateCall(callee, {val, lo, hi}, "clamp");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::string — search & match: (string, string) → bool ──────
            static const std::unordered_set<std::string> strBoolSearch = {
                "contains", "startsWith", "endsWith"
            };
            if (strBoolSearch.count(calleeName)) {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_" + calleeName, i32Ty,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret32 = builder_->CreateCall(callee, callArgs,
                    calleeName + "_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::string — search: (string, string) → int64 ─────────────
            if (calleeName == "indexOf" || calleeName == "lastIndexOf") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_" + calleeName, i64Ty,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::string — count: (string, string) → usize ──────────────
            if (calleeName == "count") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_count", usizeTy,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "count");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::string — transform: (string) → string ─────────────────
            static const std::unordered_set<std::string> strUnaryTransform = {
                "toUpper", "toLower", "trim", "trimLeft", "trimRight", "reverse"
            };
            if (strUnaryTransform.count(calleeName)) {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                // If the argument is a char (i8), skip string dispatch
                // and let the std::ascii handler below handle it
                if (!args.empty() && args[0]->getType()->isIntegerTy(8) &&
                    (calleeName == "toUpper" || calleeName == "toLower")) {
                    // Fall through to std::ascii dispatch below
                } else {
                    std::vector<llvm::Value*> callArgs;
                    extractStringArg(args[0], callArgs);
                    auto callee = declareBuiltin("lux_" + calleeName, strTy,
                        {ptrTy, usizeTy});
                    auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::string — replace/replaceFirst: (string, string, string) → string
            if (calleeName == "replace" || calleeName == "replaceFirst") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                extractStringArg(args[2], callArgs);
                auto callee = declareBuiltin("lux_" + calleeName, strTy,
                    {ptrTy, usizeTy, ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::string — repeat: (string, usize) → string ─────────────
            if (calleeName == "repeat") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* n = args[1];
                if (n->getType() != usizeTy)
                    n = builder_->CreateIntCast(n, usizeTy, false);
                callArgs.push_back(n);
                auto callee = declareBuiltin("lux_repeat", strTy,
                    {ptrTy, usizeTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "repeat");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::string — padLeft/padRight: (string, usize, char) → string
            if (calleeName == "padLeft" || calleeName == "padRight") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* width = args[1];
                if (width->getType() != usizeTy)
                    width = builder_->CreateIntCast(width, usizeTy, false);
                callArgs.push_back(width);
                auto* fill = args[2];
                if (fill->getType() != i8Ty)
                    fill = builder_->CreateIntCast(fill, i8Ty, false);
                callArgs.push_back(fill);
                auto callee = declareBuiltin("lux_" + calleeName, strTy,
                    {ptrTy, usizeTy, usizeTy, i8Ty});
                auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::string — substring: (string, usize, usize) → string ───
            if (calleeName == "substring") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* start = args[1];
                if (start->getType() != usizeTy)
                    start = builder_->CreateIntCast(start, usizeTy, false);
                callArgs.push_back(start);
                auto* length = args[2];
                if (length->getType() != usizeTy)
                    length = builder_->CreateIntCast(length, usizeTy, false);
                callArgs.push_back(length);
                auto callee = declareBuiltin("lux_substring", strTy,
                    {ptrTy, usizeTy, usizeTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "substring");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::string — charAt: (string, usize) → char ───────────────
            if (calleeName == "charAt") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* idx = args[1];
                if (idx->getType() != usizeTy)
                    idx = builder_->CreateIntCast(idx, usizeTy, false);
                callArgs.push_back(idx);
                auto callee = declareBuiltin("lux_charAt", i8Ty,
                    {ptrTy, usizeTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "charAt");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::string — slice: (string, int64, int64) → string ────────
            if (calleeName == "slice") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* start = args[1];
                if (start->getType() != i64Ty)
                    start = builder_->CreateIntCast(start, i64Ty, true);
                callArgs.push_back(start);
                auto* end = args[2];
                if (end->getType() != i64Ty)
                    end = builder_->CreateIntCast(end, i64Ty, true);
                callArgs.push_back(end);
                auto callee = declareBuiltin("lux_slice", strTy,
                    {ptrTy, usizeTy, i64Ty, i64Ty});
                auto* ret = builder_->CreateCall(callee, callArgs, "slice");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::string — parseInt: (string) → int64 ───────────────────
            if (calleeName == "parseInt") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_parseInt", i64Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "parseInt");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::string — parseIntRadix: (string, uint32) → int64 ──────
            if (calleeName == "parseIntRadix") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* radix = args[1];
                if (radix->getType() != i32Ty)
                    radix = builder_->CreateIntCast(radix, i32Ty, false);
                callArgs.push_back(radix);
                auto callee = declareBuiltin("lux_parseIntRadix", i64Ty,
                    {ptrTy, usizeTy, i32Ty});
                auto* ret = builder_->CreateCall(callee, callArgs, "parseIntRadix");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::string — parseFloat: (string) → float64 ───────────────
            if (calleeName == "parseFloat") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_parseFloat", f64Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "parseFloat");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::string — fromCharCode: (int32) → char ─────────────────
            if (calleeName == "fromCharCode") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* code = args[0];
                if (code->getType() != i32Ty)
                    code = builder_->CreateIntCast(code, i32Ty, true);
                auto callee = declareBuiltin("lux_fromCharCode", i8Ty,
                    {i32Ty});
                auto* ret = builder_->CreateCall(callee, {code}, "fromCharCode");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::mem — alloc: (usize) → *void ──────────────────────────
            if (calleeName == "alloc") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* size = args[0];
                if (size->getType() != usizeTy)
                    size = builder_->CreateIntCast(size, usizeTy, false);
                auto callee = declareBuiltin("lux_alloc", ptrTy, {usizeTy});
                auto* ret = builder_->CreateCall(callee, {size}, "alloc");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::mem — allocZeroed: (usize) → *void ────────────────────
            if (calleeName == "allocZeroed") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* size = args[0];
                if (size->getType() != usizeTy)
                    size = builder_->CreateIntCast(size, usizeTy, false);
                auto callee = declareBuiltin("lux_allocZeroed", ptrTy, {usizeTy});
                auto* ret = builder_->CreateCall(callee, {size}, "allocZeroed");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::mem — realloc: (*void, usize) → *void ─────────────────
            if (calleeName == "realloc") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* ptr  = args[0];
                auto* size = args[1];
                if (size->getType() != usizeTy)
                    size = builder_->CreateIntCast(size, usizeTy, false);
                auto callee = declareBuiltin("lux_realloc", ptrTy,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, {ptr, size}, "realloc");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::mem — compare: (*void, *void, usize) → int32 ──────────
            if (calleeName == "compare") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* a = args[0];
                auto* b = args[1];
                auto* n = args[2];
                if (n->getType() != usizeTy)
                    n = builder_->CreateIntCast(n, usizeTy, false);
                auto callee = declareBuiltin("lux_compare", i32Ty,
                    {ptrTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, {a, b, n}, "compare");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::random — randInt: () → int64 ──────────────────────────
            if (calleeName == "randInt") {
                auto callee = declareBuiltin("lux_randInt", i64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "randInt");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::random — randIntRange: (int64, int64) → int64 ─────────
            if (calleeName == "randIntRange") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* lo = args[0];
                auto* hi = args[1];
                if (lo->getType() != i64Ty)
                    lo = builder_->CreateIntCast(lo, i64Ty, true);
                if (hi->getType() != i64Ty)
                    hi = builder_->CreateIntCast(hi, i64Ty, true);
                auto callee = declareBuiltin("lux_randIntRange", i64Ty,
                    {i64Ty, i64Ty});
                auto* ret = builder_->CreateCall(callee, {lo, hi}, "randIntRange");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::random — randUint: () → uint64 ────────────────────────
            if (calleeName == "randUint") {
                auto callee = declareBuiltin("lux_randUint", i64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "randUint");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::random — randFloat: () → float64 ──────────────────────
            if (calleeName == "randFloat") {
                auto callee = declareBuiltin("lux_randFloat", f64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "randFloat");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::random — randFloatRange: (float64, float64) → float64 ─
            if (calleeName == "randFloatRange") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* lo = args[0];
                auto* hi = args[1];
                if (lo->getType() != f64Ty)
                    lo = builder_->CreateFPCast(lo, f64Ty);
                if (hi->getType() != f64Ty)
                    hi = builder_->CreateFPCast(hi, f64Ty);
                auto callee = declareBuiltin("lux_randFloatRange", f64Ty,
                    {f64Ty, f64Ty});
                auto* ret = builder_->CreateCall(callee, {lo, hi}, "randFloatRange");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::random — randBool: () → bool ──────────────────────────
            if (calleeName == "randBool") {
                auto callee = declareBuiltin("lux_randBool", i32Ty, {});
                auto* ret32 = builder_->CreateCall(callee, {}, "randBool_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "randBool");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::random — randChar: () → char ──────────────────────────
            if (calleeName == "randChar") {
                auto callee = declareBuiltin("lux_randChar", i8Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "randChar");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::time — no-arg, returns uint64 ─────────────────────────
            if (calleeName == "now") {
                auto callee = declareBuiltin("lux_now", i64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "now");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "nowNanos") {
                auto callee = declareBuiltin("lux_nowNanos", i64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "nowNanos");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "nowMicros") {
                auto callee = declareBuiltin("lux_nowMicros", i64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "nowMicros");
                return static_cast<llvm::Value*>(ret);
            }
            if (calleeName == "clock") {
                auto callee = declareBuiltin("lux_clock", i64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "clock");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::time — no-arg, returns int32 ──────────────────────────
            static const std::unordered_set<std::string> timeI32Funcs = {
                "year", "month", "day", "hour", "minute", "second", "weekday"
            };
            if (timeI32Funcs.count(calleeName)) {
                auto callee = declareBuiltin("lux_" + calleeName, i32Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::time — timestamp: () → string ─────────────────────────
            if (calleeName == "timestamp") {
                auto callee = declareBuiltin("lux_timestamp", strTy, {});
                auto* ret = builder_->CreateCall(callee, {}, "timestamp");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::time — elapsed: (uint64) → uint64 ─────────────────────
            if (calleeName == "elapsed") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* since = args[0];
                if (since->getType() != i64Ty)
                    since = builder_->CreateIntCast(since, i64Ty, false);
                auto callee = declareBuiltin("lux_elapsed", i64Ty, {i64Ty});
                auto* ret = builder_->CreateCall(callee, {since}, "elapsed");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::time — formatTime: (uint64, string) → string ──────────
            if (calleeName == "formatTime") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* ms = args[0];
                if (ms->getType() != i64Ty)
                    ms = builder_->CreateIntCast(ms, i64Ty, false);
                auto* fmtPtr = builder_->CreateExtractValue(args[1], 0, "fmt_ptr");
                auto* fmtLen = builder_->CreateExtractValue(args[1], 1, "fmt_len");
                auto callee = declareBuiltin("lux_formatTime", strTy,
                    {i64Ty, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee,
                    {ms, fmtPtr, fmtLen}, "formatTime");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::time — parseTime: (string, string) → uint64 ───────────
            if (calleeName == "parseTime") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                auto* strPtr = builder_->CreateExtractValue(args[0], 0, "str_ptr");
                auto* strLen = builder_->CreateExtractValue(args[0], 1, "str_len");
                auto* fmtPtr = builder_->CreateExtractValue(args[1], 0, "fmt_ptr");
                auto* fmtLen = builder_->CreateExtractValue(args[1], 1, "fmt_len");
                auto callee = declareBuiltin("lux_parseTime", i64Ty,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee,
                    {strPtr, strLen, fmtPtr, fmtLen}, "parseTime");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::fs — no-arg string returns: cwd, tempDir ──────────────
            if (calleeName == "cwd" || calleeName == "tempDir") {
                auto callee = declareBuiltin("lux_" + calleeName, strTy, {});
                auto* ret = builder_->CreateCall(callee, {}, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::fs — readFile: (string) → string ──────────────────────
            if (calleeName == "readFile") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_readFile", strTy,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "readFile");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::fs — single-string-arg bool returns ────────────────────
            {
                static const std::unordered_set<std::string> fsBoolFuncs1 = {
                    "exists", "isFile", "isDir",
                    "remove", "removeDir",
                    "mkdir", "mkdirAll", "setCwd"
                };
                if (fsBoolFuncs1.count(calleeName)) {
                    std::vector<llvm::Value*> args;
                    if (auto* argList = ctx->argList()) {
                        for (auto* exprCtx : argList->expression())
                            args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                    }
                    std::vector<llvm::Value*> callArgs;
                    extractStringArg(args[0], callArgs);
                    std::string cName = "lux_" + calleeName;
                    auto callee = declareBuiltin(cName, i32Ty,
                        {ptrTy, usizeTy});
                    auto* ret32 = builder_->CreateCall(callee, callArgs, calleeName + "_i32");
                    auto* ret = builder_->CreateICmpNE(ret32,
                        llvm::ConstantInt::get(i32Ty, 0), calleeName);
                    return static_cast<llvm::Value*>(ret);
                }
            }

            // ── std::fs — rename: (string, string) → bool ──────────────────
            if (calleeName == "rename") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_fsRename", i32Ty,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret32 = builder_->CreateCall(callee, callArgs, "rename_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "rename");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::fs — fileSize: (string) → int64 ───────────────────────
            if (calleeName == "fileSize") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_fileSize", i64Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "fileSize");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::process — no-arg string returns ────────────────────────
            {
                static const std::unordered_set<std::string> procStrNoArg = {
                    "platform", "arch", "homeDir", "executablePath"
                };
                if (procStrNoArg.count(calleeName)) {
                    auto callee = declareBuiltin("lux_" + calleeName, strTy, {});
                    auto* ret = builder_->CreateCall(callee, {}, calleeName);
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::process — env: (string) → string ──────────────────────
            if (calleeName == "env") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_env", strTy,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "env");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::process — execOutput: (string) → string ───────────────
            if (calleeName == "execOutput") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_execOutput", strTy,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "execOutput");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::process — hasEnv: (string) → bool ─────────────────────
            if (calleeName == "hasEnv") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_hasEnv", i32Ty,
                    {ptrTy, usizeTy});
                auto* ret32 = builder_->CreateCall(callee, callArgs, "hasEnv_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "hasEnv");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::process — exec: (string) → int32 ──────────────────────
            if (calleeName == "exec") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList()) {
                    for (auto* exprCtx : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(exprCtx)));
                }
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_exec", i32Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "exec");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::process — pid: () → int32 ─────────────────────────────
            if (calleeName == "pid") {
                auto callee = declareBuiltin("lux_pid", i32Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "pid");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::conv — itoa: (int64) → string ─────────────────────────
            if (calleeName == "itoa") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, true);
                auto callee = declareBuiltin("lux_itoa", strTy, {i64Ty});
                auto* ret = builder_->CreateCall(callee, {a}, "itoa");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::conv — itoaRadix: (int64, uint32) → string ────────────
            if (calleeName == "itoaRadix") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* val = args[0];
                if (val->getType() != i64Ty)
                    val = builder_->CreateIntCast(val, i64Ty, true);
                auto* radix = args[1];
                if (radix->getType() != i32Ty)
                    radix = builder_->CreateIntCast(radix, i32Ty, false);
                auto callee = declareBuiltin("lux_itoaRadix", strTy, {i64Ty, i32Ty});
                auto* ret = builder_->CreateCall(callee, {val, radix}, "itoaRadix");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::conv — utoa: (uint64) → string ────────────────────────
            if (calleeName == "utoa") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                auto callee = declareBuiltin("lux_utoa", strTy, {i64Ty});
                auto* ret = builder_->CreateCall(callee, {a}, "utoa");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::conv — ftoa: (float64) → string ───────────────────────
            if (calleeName == "ftoa") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != f64Ty)
                    a = builder_->CreateFPExt(a, f64Ty);
                auto callee = declareBuiltin("lux_ftoa", strTy, {f64Ty});
                auto* ret = builder_->CreateCall(callee, {a}, "ftoa");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::conv — ftoaPrecision: (float64, uint32) → string ──────
            if (calleeName == "ftoaPrecision") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* val = args[0];
                if (val->getType() != f64Ty)
                    val = builder_->CreateFPExt(val, f64Ty);
                auto* prec = args[1];
                if (prec->getType() != i32Ty)
                    prec = builder_->CreateIntCast(prec, i32Ty, false);
                auto callee = declareBuiltin("lux_ftoaPrecision", strTy, {f64Ty, i32Ty});
                auto* ret = builder_->CreateCall(callee, {val, prec}, "ftoaPrecision");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::conv — toHex/toOctal/toBinary: (uint64) → string ──────
            if (calleeName == "toHex" || calleeName == "toOctal" ||
                calleeName == "toBinary") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                auto callee = declareBuiltin("lux_" + calleeName, strTy, {i64Ty});
                auto* ret = builder_->CreateCall(callee, {a}, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::conv — atoi: (string) → int64 ─────────────────────────
            if (calleeName == "atoi") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_atoi", i64Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "atoi");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::conv — atof: (string) → float64 ───────────────────────
            if (calleeName == "atof") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_atof", f64Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "atof");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::conv — fromHex: (string) → uint64 ─────────────────────
            if (calleeName == "fromHex") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_fromHex", i64Ty,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "fromHex");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::conv — charToInt: (char) → int32 ──────────────────────
            if (calleeName == "charToInt") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto callee = declareBuiltin("lux_charToInt", i32Ty, {i8Ty});
                auto* ret = builder_->CreateCall(callee, {args[0]}, "charToInt");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::conv — intToChar: (int32) → char ──────────────────────
            if (calleeName == "intToChar") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i32Ty)
                    a = builder_->CreateIntCast(a, i32Ty, true);
                auto callee = declareBuiltin("lux_intToChar", i8Ty, {i32Ty});
                auto* ret = builder_->CreateCall(callee, {a}, "intToChar");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::hash — hashString: (string) → uint64 ─────────────────
            if (calleeName == "hashString") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_hashString", i64Ty, {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "hashString");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::hash — hashInt: (int64) → uint64 ─────────────────────
            if (calleeName == "hashInt") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, true);
                auto callee = declareBuiltin("lux_hashInt", i64Ty, {i64Ty});
                auto* ret = builder_->CreateCall(callee, {a}, "hashInt");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::hash — hashCombine: (uint64, uint64) → uint64 ────────
            if (calleeName == "hashCombine") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                auto* b = args[1];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                if (b->getType() != i64Ty)
                    b = builder_->CreateIntCast(b, i64Ty, false);
                auto callee = declareBuiltin("lux_hashCombine", i64Ty, {i64Ty, i64Ty});
                auto* ret = builder_->CreateCall(callee, {a, b}, "hashCombine");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::bits — (uint64) → uint32: popcount, ctz, clz ────────
            if (calleeName == "popcount" || calleeName == "ctz" ||
                calleeName == "clz") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                auto callee = declareBuiltin("lux_" + calleeName, i32Ty, {i64Ty});
                auto* ret = builder_->CreateCall(callee, {a}, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::bits — (uint64, uint32) → uint64: rotl, rotr,
            //    setBit, clearBit, toggleBit ───────────────────────────────
            if (calleeName == "rotl" || calleeName == "rotr" ||
                calleeName == "setBit" || calleeName == "clearBit" ||
                calleeName == "toggleBit") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                auto* b = args[1];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                if (b->getType() != i32Ty)
                    b = builder_->CreateIntCast(b, i32Ty, false);
                auto callee = declareBuiltin("lux_" + calleeName, i64Ty, {i64Ty, i32Ty});
                auto* ret = builder_->CreateCall(callee, {a, b}, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::bits — (uint64) → uint64: bswap, nextPow2,
            //    bitReverse ────────────────────────────────────────────────
            if (calleeName == "bswap" || calleeName == "nextPow2" ||
                calleeName == "bitReverse") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                auto callee = declareBuiltin("lux_" + calleeName, i64Ty, {i64Ty});
                auto* ret = builder_->CreateCall(callee, {a}, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::bits — isPow2: (uint64) → bool ──────────────────────
            if (calleeName == "isPow2") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                auto callee = declareBuiltin("lux_isPow2", i32Ty, {i64Ty});
                auto* raw = builder_->CreateCall(callee, {a}, "isPow2_raw");
                auto* ret = builder_->CreateICmpNE(raw,
                    llvm::ConstantInt::get(i32Ty, 0), "isPow2");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::bits — testBit: (uint64, uint32) → bool ─────────────
            if (calleeName == "testBit") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                auto* b = args[1];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                if (b->getType() != i32Ty)
                    b = builder_->CreateIntCast(b, i32Ty, false);
                auto callee = declareBuiltin("lux_testBit", i32Ty, {i64Ty, i32Ty});
                auto* raw = builder_->CreateCall(callee, {a, b}, "testBit_raw");
                auto* ret = builder_->CreateICmpNE(raw,
                    llvm::ConstantInt::get(i32Ty, 0), "testBit");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::bits — extractBits: (uint64, uint32, uint32) → uint64
            if (calleeName == "extractBits") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                auto* b = args[1];
                auto* c = args[2];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, false);
                if (b->getType() != i32Ty)
                    b = builder_->CreateIntCast(b, i32Ty, false);
                if (c->getType() != i32Ty)
                    c = builder_->CreateIntCast(c, i32Ty, false);
                auto callee = declareBuiltin("lux_extractBits", i64Ty,
                    {i64Ty, i32Ty, i32Ty});
                auto* ret = builder_->CreateCall(callee, {a, b, c}, "extractBits");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::char — (char) → bool classification ─────────────────
            if (calleeName == "isAlpha" || calleeName == "isDigit" ||
                calleeName == "isAlphaNum" || calleeName == "isUpper" ||
                calleeName == "isLower" || calleeName == "isWhitespace" ||
                calleeName == "isPrintable" || calleeName == "isControl" ||
                calleeName == "isHexDigit" || calleeName == "isAscii") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                auto* i8Ty = llvm::Type::getInt8Ty(*context_);
                if (a->getType() != i8Ty)
                    a = builder_->CreateIntCast(a, i8Ty, false);
                auto callee = declareBuiltin("lux_" + calleeName, i32Ty, {i8Ty});
                auto* raw = builder_->CreateCall(callee, {a}, calleeName + "_raw");
                auto* ret = builder_->CreateICmpNE(raw,
                    llvm::ConstantInt::get(i32Ty, 0), calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::char — toUpper/toLower: (char) → char ───────────────
            if (calleeName == "toUpper" || calleeName == "toLower") {
                // Check if arg is char (not string) — std::char version
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                auto* i8Ty = llvm::Type::getInt8Ty(*context_);
                if (a->getType() == i8Ty ||
                    (a->getType()->isIntegerTy() &&
                     a->getType()->getIntegerBitWidth() == 8)) {
                    if (a->getType() != i8Ty)
                        a = builder_->CreateIntCast(a, i8Ty, false);
                    auto callee = declareBuiltin(
                        "lux_char_" + calleeName, i8Ty, {i8Ty});
                    auto* ret = builder_->CreateCall(callee, {a}, calleeName);
                    return static_cast<llvm::Value*>(ret);
                }
                // Fall through to string version (std::str)
            }

            // ── std::char — toDigit: (char) → int32 ──────────────────────
            if (calleeName == "toDigit") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                auto* i8Ty = llvm::Type::getInt8Ty(*context_);
                if (a->getType() != i8Ty)
                    a = builder_->CreateIntCast(a, i8Ty, false);
                auto callee = declareBuiltin("lux_toDigit", i32Ty, {i8Ty});
                auto* ret = builder_->CreateCall(callee, {a}, "toDigit");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::char — fromDigit: (int32) → char ────────────────────
            if (calleeName == "fromDigit") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i32Ty)
                    a = builder_->CreateIntCast(a, i32Ty, true);
                auto* i8Ty = llvm::Type::getInt8Ty(*context_);
                auto callee = declareBuiltin("lux_fromDigit", i8Ty, {i32Ty});
                auto* ret = builder_->CreateCall(callee, {a}, "fromDigit");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::path — join: (string, string) → string ─────────────
            if (calleeName == "join") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                if (args.size() == 2 && args[0]->getType() == strTy) {
                    std::vector<llvm::Value*> callArgs;
                    extractStringArg(args[0], callArgs);
                    extractStringArg(args[1], callArgs);
                    auto callee = declareBuiltin("lux_pathJoin", strTy,
                        {ptrTy, usizeTy, ptrTy, usizeTy});
                    auto* ret = builder_->CreateCall(callee, callArgs, "pathJoin");
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::path — single-string → string: parent, fileName, stem,
            //    extension, normalize, toAbsolute ─────────────────────────
            {
                static const std::unordered_set<std::string> pathStrFuncs = {
                    "parent", "fileName", "stem", "extension",
                    "normalize", "toAbsolute"
                };
                if (pathStrFuncs.count(calleeName)) {
                    std::vector<llvm::Value*> args;
                    if (auto* argList = ctx->argList())
                        for (auto* e : argList->expression())
                            args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                    std::vector<llvm::Value*> callArgs;
                    extractStringArg(args[0], callArgs);
                    auto callee = declareBuiltin("lux_" + calleeName, strTy,
                        {ptrTy, usizeTy});
                    auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::path — single-string → bool: isAbsolute, isRelative ─
            if (calleeName == "isAbsolute" || calleeName == "isRelative") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_" + calleeName, i32Ty,
                    {ptrTy, usizeTy});
                auto* ret32 = builder_->CreateCall(callee, callArgs, calleeName + "_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::path — separator: () → char ─────────────────────────
            if (calleeName == "separator") {
                auto callee = declareBuiltin("lux_separator", i8Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "separator");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::path — (string, string) → string: withExtension, withFileName
            if (calleeName == "withExtension" || calleeName == "withFileName") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_" + calleeName, strTy,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::fmt — lpad/rpad/center: (string, usize, char) → string
            if (calleeName == "lpad" || calleeName == "rpad" ||
                calleeName == "center") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* width = args[1];
                if (width->getType() != usizeTy)
                    width = builder_->CreateIntCast(width, usizeTy, false);
                callArgs.push_back(width);
                auto* fill = args[2];
                if (fill->getType() != i8Ty)
                    fill = builder_->CreateIntCast(fill, i8Ty, false);
                callArgs.push_back(fill);
                auto callee = declareBuiltin("lux_" + calleeName, strTy,
                    {ptrTy, usizeTy, usizeTy, i8Ty});
                auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::fmt — hex/hexUpper/oct/bin/humanBytes: (uint64) → string
            {
                static const std::unordered_map<std::string, std::string> fmtU64Funcs = {
                    {"hex", "lux_fmtHex"}, {"hexUpper", "lux_fmtHexUpper"},
                    {"oct", "lux_fmtOct"}, {"bin", "lux_fmtBin"},
                    {"humanBytes", "lux_humanBytes"}
                };
                auto fmtIt = fmtU64Funcs.find(calleeName);
                if (fmtIt != fmtU64Funcs.end()) {
                    std::vector<llvm::Value*> args;
                    if (auto* argList = ctx->argList())
                        for (auto* e : argList->expression())
                            args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                    auto* a = args[0];
                    auto* u64Ty = llvm::Type::getInt64Ty(*context_);
                    if (a->getType() != u64Ty)
                        a = builder_->CreateIntCast(a, u64Ty, false);
                    auto callee = declareBuiltin(fmtIt->second, strTy, {u64Ty});
                    auto* ret = builder_->CreateCall(callee, {a}, calleeName);
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::fmt — commas: (int64) → string ─────────────────────
            if (calleeName == "commas") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = args[0];
                if (a->getType() != i64Ty)
                    a = builder_->CreateIntCast(a, i64Ty, true);
                auto callee = declareBuiltin("lux_commas", strTy, {i64Ty});
                auto* ret = builder_->CreateCall(callee, {a}, "commas");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::fmt — fixed: (float64, uint32) → string ────────────
            if (calleeName == "fixed") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* val = args[0];
                if (val->getType() != f64Ty)
                    val = builder_->CreateSIToFP(val, f64Ty);
                auto* dec = args[1];
                auto* u32Ty = llvm::Type::getInt32Ty(*context_);
                if (dec->getType() != u32Ty)
                    dec = builder_->CreateIntCast(dec, u32Ty, false);
                auto callee = declareBuiltin("lux_fixed", strTy,
                    {f64Ty, u32Ty});
                auto* ret = builder_->CreateCall(callee, {val, dec}, "fixed");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::fmt — scientific/percent: (float64) → string ───────
            if (calleeName == "scientific" || calleeName == "percent") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* val = args[0];
                if (val->getType() != f64Ty)
                    val = builder_->CreateSIToFP(val, f64Ty);
                auto callee = declareBuiltin("lux_" + calleeName, strTy,
                    {f64Ty});
                auto* ret = builder_->CreateCall(callee, {val}, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::regex — match: (string, string) → bool ─────────────
            if (calleeName == "match") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_regexMatch", i32Ty,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret32 = builder_->CreateCall(callee, callArgs, "match_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "match");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::regex — find: (string, string) → string ────────────
            if (calleeName == "find") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_regexFind", strTy,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "find");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::regex — findIndex: (string, string) → int64 ────────
            if (calleeName == "findIndex") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_regexFindIndex", i64Ty,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "findIndex");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::regex — regexReplace/regexReplaceFirst: (string, string, string) → string
            if (calleeName == "regexReplace" || calleeName == "regexReplaceFirst") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                extractStringArg(args[2], callArgs);
                auto callee = declareBuiltin("lux_" + calleeName, strTy,
                    {ptrTy, usizeTy, ptrTy, usizeTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::regex — isValid: (string) → bool ───────────────────
            if (calleeName == "isValid") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_regexIsValid", i32Ty,
                    {ptrTy, usizeTy});
                auto* ret32 = builder_->CreateCall(callee, callArgs, "isValid_i32");
                auto* ret = builder_->CreateICmpNE(ret32,
                    llvm::ConstantInt::get(i32Ty, 0), "isValid");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::encoding — string → string functions ────────────────
            {
                static const std::unordered_map<std::string, std::string> encStrFuncs = {
                    {"base64EncodeStr", "lux_base64EncodeStr"},
                    {"base64DecodeStr", "lux_base64DecodeStr"},
                    {"urlEncode",       "lux_urlEncode"},
                    {"urlDecode",       "lux_urlDecode"},
                };
                auto encIt = encStrFuncs.find(calleeName);
                if (encIt != encStrFuncs.end()) {
                    std::vector<llvm::Value*> args;
                    if (auto* argList = ctx->argList())
                        for (auto* e : argList->expression())
                            args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                    std::vector<llvm::Value*> callArgs;
                    extractStringArg(args[0], callArgs);
                    auto callee = declareBuiltin(encIt->second, strTy,
                        {ptrTy, usizeTy});
                    auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::crypto — string → string hash functions ─────────────
            {
                static const std::unordered_map<std::string, std::string> cryptoStrFuncs = {
                    {"md5String",    "lux_md5String"},
                    {"sha1String",   "lux_sha1String"},
                    {"sha256String", "lux_sha256String"},
                    {"sha512String", "lux_sha512String"},
                };
                auto crIt = cryptoStrFuncs.find(calleeName);
                if (crIt != cryptoStrFuncs.end()) {
                    std::vector<llvm::Value*> args;
                    if (auto* argList = ctx->argList())
                        for (auto* e : argList->expression())
                            args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                    std::vector<llvm::Value*> callArgs;
                    extractStringArg(args[0], callArgs);
                    auto callee = declareBuiltin(crIt->second, strTy,
                        {ptrTy, usizeTy});
                    auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::compress — string → string (gzip/deflate) ────────
            {
                static const std::unordered_map<std::string, std::string> compressStrFuncs = {
                    {"gzipCompress",   "lux_gzipCompress"},
                    {"gzipDecompress", "lux_gzipDecompress"},
                    {"deflate",        "lux_deflate"},
                    {"inflate",        "lux_inflate"},
                };
                auto cpIt = compressStrFuncs.find(calleeName);
                if (cpIt != compressStrFuncs.end()) {
                    std::vector<llvm::Value*> args;
                    if (auto* argList = ctx->argList())
                        for (auto* e : argList->expression())
                            args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                    std::vector<llvm::Value*> callArgs;
                    extractStringArg(args[0], callArgs);
                    auto callee = declareBuiltin(cpIt->second, strTy,
                        {ptrTy, usizeTy});
                    auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::compress — compressLevel: (string, int32) → string ──
            if (calleeName == "compressLevel") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* lvl = args[1];
                if (lvl->getType() != i32Ty)
                    lvl = builder_->CreateIntCast(lvl, i32Ty, true);
                callArgs.push_back(lvl);
                auto callee = declareBuiltin("lux_compressLevel", strTy,
                    {ptrTy, usizeTy, i32Ty});
                auto* ret = builder_->CreateCall(callee, callArgs, "compressLevel");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::net — resolve: string → string ─────────────────────
            if (calleeName == "resolve") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_netResolve", strTy,
                    {ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "resolve");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::thread — cpuCount: () → uint32 ────────────────────
            if (calleeName == "cpuCount") {
                auto* u32Ty = llvm::Type::getInt32Ty(*context_);
                auto callee = declareBuiltin("lux_cpuCount", u32Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "cpuCount");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::thread — threadId: () → uint64 ────────────────────
            if (calleeName == "threadId") {
                auto callee = declareBuiltin("lux_threadId", i64Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "threadId");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::net — tcpConnect/tcpListen/udpBind: (string, uint16) → int32
            if (calleeName == "tcpConnect" || calleeName == "tcpListen" ||
                calleeName == "udpBind") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                std::vector<llvm::Value*> callArgs;
                extractStringArg(args[0], callArgs);
                auto* i16Ty = llvm::Type::getInt16Ty(*context_);
                auto* port = builder_->CreateIntCast(args[1], i16Ty, false);
                callArgs.push_back(port);
                std::string cName = "lux_" + calleeName;
                auto callee = declareBuiltin(cName, i32Ty,
                    {ptrTy, usizeTy, i16Ty});
                auto* ret = builder_->CreateCall(callee, callArgs, calleeName);
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::net — tcpAccept: (int32) → int32 ────────────────────
            if (calleeName == "tcpAccept") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* fd = builder_->CreateIntCast(args[0], i32Ty, true);
                auto callee = declareBuiltin("lux_tcpAccept", i32Ty, {i32Ty});
                auto* ret = builder_->CreateCall(callee, {fd}, "tcpAccept");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::net — tcpSend: (int32, string) → int64 ──────────────
            if (calleeName == "tcpSend") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* fd = builder_->CreateIntCast(args[0], i32Ty, true);
                std::vector<llvm::Value*> callArgs;
                callArgs.push_back(fd);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_tcpSend", i64Ty,
                    {i32Ty, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "tcpSend");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::net — tcpRecv/udpRecvFrom: (int32, usize) → string ──
            if (calleeName == "tcpRecv" || calleeName == "udpRecvFrom") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* fd = builder_->CreateIntCast(args[0], i32Ty, true);
                auto* maxLen = args[1];
                if (maxLen->getType() != usizeTy)
                    maxLen = builder_->CreateIntCast(maxLen, usizeTy, false);
                std::string cName = "lux_" + calleeName;
                auto callee = declareBuiltin(cName, strTy,
                    {i32Ty, usizeTy});
                auto* ret = builder_->CreateCall(callee, {fd, maxLen}, calleeName);
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::net — udpSendTo: (int32, string, string, uint16) → int64
            if (calleeName == "udpSendTo") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* fd = builder_->CreateIntCast(args[0], i32Ty, true);
                std::vector<llvm::Value*> callArgs;
                callArgs.push_back(fd);
                extractStringArg(args[1], callArgs);  // data
                extractStringArg(args[2], callArgs);  // host
                auto* i16Ty = llvm::Type::getInt16Ty(*context_);
                auto* port = builder_->CreateIntCast(args[3], i16Ty, false);
                callArgs.push_back(port);
                auto callee = declareBuiltin("lux_udpSendTo", i64Ty,
                    {i32Ty, ptrTy, usizeTy, ptrTy, usizeTy, i16Ty});
                auto* ret = builder_->CreateCall(callee, callArgs, "udpSendTo");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::os — no-arg → int32 ─────────────────────────────────
            {
                static const std::unordered_map<std::string, std::string> osI32NoArg = {
                    {"getpid",  "lux_osGetpid"},
                    {"getppid", "lux_osGetppid"},
                };
                auto osIt = osI32NoArg.find(calleeName);
                if (osIt != osI32NoArg.end()) {
                    auto callee = declareBuiltin(osIt->second, i32Ty, {});
                    auto* ret = builder_->CreateCall(callee, {}, calleeName);
                    return static_cast<llvm::Value*>(ret);
                }
            }

            // ── std::os — no-arg → uint32 ────────────────────────────────
            {
                static const std::unordered_map<std::string, std::string> osU32NoArg = {
                    {"getuid", "lux_osGetuid"},
                    {"getgid", "lux_osGetgid"},
                };
                auto osIt = osU32NoArg.find(calleeName);
                if (osIt != osU32NoArg.end()) {
                    auto u32Ty = llvm::Type::getInt32Ty(*context_);
                    auto callee = declareBuiltin(osIt->second, u32Ty, {});
                    auto* ret = builder_->CreateCall(callee, {}, calleeName);
                    return static_cast<llvm::Value*>(ret);
                }
            }

            // ── std::os — hostname: () → string ──────────────────────────
            if (calleeName == "hostname") {
                auto callee = declareBuiltin("lux_osHostname", strTy, {});
                auto* ret = builder_->CreateCall(callee, {}, "hostname");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::os — pageSize: () → usize ──────────────────────────
            if (calleeName == "pageSize") {
                auto callee = declareBuiltin("lux_osPageSize", usizeTy, {});
                auto* ret = builder_->CreateCall(callee, {}, "pageSize");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::os — errno: () → int32 ─────────────────────────────
            if (calleeName == "errno") {
                auto callee = declareBuiltin("lux_osErrno", i32Ty, {});
                auto* ret = builder_->CreateCall(callee, {}, "osErrno");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::os — strerror: (int32) → string ────────────────────
            if (calleeName == "strerror") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* code = builder_->CreateIntCast(args[0], i32Ty, true);
                auto callee = declareBuiltin("lux_osStrerror", strTy, {i32Ty});
                auto* ret = builder_->CreateCall(callee, {code}, "strerror");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::os — kill: (int32, int32) → int32 ──────────────────
            if (calleeName == "kill") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = builder_->CreateIntCast(args[0], i32Ty, true);
                auto* b = builder_->CreateIntCast(args[1], i32Ty, true);
                auto callee = declareBuiltin("lux_osKill", i32Ty, {i32Ty, i32Ty});
                auto* ret = builder_->CreateCall(callee, {a, b}, "kill");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::os — fd ops: (int32) → int32 ───────────────────────
            {
                static const std::unordered_map<std::string, std::string> osFdOps = {
                    {"dup",     "lux_osDup"},
                    {"closeFd", "lux_osCloseFd"},
                };
                auto osIt = osFdOps.find(calleeName);
                if (osIt != osFdOps.end()) {
                    std::vector<llvm::Value*> args;
                    if (auto* argList = ctx->argList())
                        for (auto* e : argList->expression())
                            args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                    auto* fd = builder_->CreateIntCast(args[0], i32Ty, true);
                    auto callee = declareBuiltin(osIt->second, i32Ty, {i32Ty});
                    auto* ret = builder_->CreateCall(callee, {fd}, calleeName);
                    return static_cast<llvm::Value*>(ret);
                }
            }

            // ── std::os — dup2: (int32, int32) → int32 ──────────────────
            if (calleeName == "dup2") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* a = builder_->CreateIntCast(args[0], i32Ty, true);
                auto* b = builder_->CreateIntCast(args[1], i32Ty, true);
                auto callee = declareBuiltin("lux_osDup2", i32Ty, {i32Ty, i32Ty});
                auto* ret = builder_->CreateCall(callee, {a, b}, "dup2");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::io — readLines: () → Vec<string> ──────────────────────
            if (calleeName == "readLines") {
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "readLines_out");
                auto callee = declareBuiltin("lux_readLines", voidTy, {ptrTy});
                builder_->CreateCall(callee, {outAlloca});
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "readLines_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::io — readNBytes: (usize) → Vec<uint8> ─────────────────
            if (calleeName == "readNBytes") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "readNBytes_out");
                auto* n = args[0];
                if (n->getType() != usizeTy)
                    n = builder_->CreateIntCast(n, usizeTy, false);
                auto callee = declareBuiltin("lux_readNBytes", voidTy, {ptrTy, usizeTy});
                builder_->CreateCall(callee, {outAlloca, n});
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "readNBytes_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::str — split: (string, string) → Vec<string> ───────────
            if (calleeName == "split") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "split_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_split", voidTy,
                    {ptrTy, ptrTy, usizeTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "split_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::str — splitN: (string, string, usize) → Vec<string> ───
            if (calleeName == "splitN") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "splitN_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto* n = args[2];
                if (n->getType() != usizeTy)
                    n = builder_->CreateIntCast(n, usizeTy, false);
                callArgs.push_back(n);
                auto callee = declareBuiltin("lux_splitN", voidTy,
                    {ptrTy, ptrTy, usizeTy, ptrTy, usizeTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "splitN_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::str — joinVec: (Vec<string>, string) → string ─────────
            if (calleeName == "joinVec") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy       = getOrCreateVecStructType();
                auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, "joinVec_arg");
                builder_->CreateStore(args[0], vecArgAlloca);
                std::vector<llvm::Value*> callArgs = {vecArgAlloca};
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_joinVec", strTy,
                    {ptrTy, ptrTy, usizeTy});
                auto* ret = builder_->CreateCall(callee, callArgs, "joinVec");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::str — lines: (string) → Vec<string> ───────────────────
            if (calleeName == "lines") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "lines_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_lines", voidTy,
                    {ptrTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "lines_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::str — chars: (string) → Vec<char> ─────────────────────
            if (calleeName == "chars") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "chars_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_chars", voidTy,
                    {ptrTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "chars_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::str — fromChars: (Vec<char>) → string ─────────────────
            if (calleeName == "fromChars") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy       = getOrCreateVecStructType();
                auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, "fromChars_arg");
                builder_->CreateStore(args[0], vecArgAlloca);
                auto callee = declareBuiltin("lux_fromCharsVec", strTy, {ptrTy});
                auto* ret = builder_->CreateCall(callee, {vecArgAlloca}, "fromChars");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::str — toBytes: (string) → Vec<uint8> ──────────────────
            if (calleeName == "toBytes") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "toBytes_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_toBytes", voidTy,
                    {ptrTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "toBytes_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::str — fromBytes: (Vec<uint8>) → string ────────────────
            if (calleeName == "fromBytes") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy       = getOrCreateVecStructType();
                auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, "fromBytes_arg");
                builder_->CreateStore(args[0], vecArgAlloca);
                auto callee = declareBuiltin("lux_fromBytesVec", strTy, {ptrTy});
                auto* ret = builder_->CreateCall(callee, {vecArgAlloca}, "fromBytes");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::fs — listDir: (string) → Vec<string> ──────────────────
            if (calleeName == "listDir") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "listDir_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_listDir", voidTy,
                    {ptrTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "listDir_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::fs — readBytes: (string) → Vec<uint8> ─────────────────
            if (calleeName == "readBytes") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "readBytes_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_readFileBytes", voidTy,
                    {ptrTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "readBytes_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::hash — hashBytes: (Vec<uint8>) → uint64 ───────────────
            if (calleeName == "hashBytes") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy       = getOrCreateVecStructType();
                auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, "hashBytes_arg");
                builder_->CreateStore(args[0], vecArgAlloca);
                auto callee = declareBuiltin("lux_hashBytesVec", i64Ty, {ptrTy});
                auto* ret = builder_->CreateCall(callee, {vecArgAlloca}, "hashBytes");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::hash — crc32: (Vec<uint8>) → uint32 ───────────────────
            if (calleeName == "crc32") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy       = getOrCreateVecStructType();
                auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, "crc32_arg");
                builder_->CreateStore(args[0], vecArgAlloca);
                auto callee = declareBuiltin("lux_crc32Bytes", i32Ty, {ptrTy});
                auto* ret = builder_->CreateCall(callee, {vecArgAlloca}, "crc32");
                return static_cast<llvm::Value*>(ret);
            }

            // ── std::regex — findAll: (string, string) → Vec<string> ───────
            if (calleeName == "findAll") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "findAll_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_regexFindAll", voidTy,
                    {ptrTy, ptrTy, usizeTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "findAll_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::regex — regexSplit: (string, string) → Vec<string> ────
            if (calleeName == "regexSplit") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "regexSplit_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                extractStringArg(args[1], callArgs);
                auto callee = declareBuiltin("lux_regexSplit", voidTy,
                    {ptrTy, ptrTy, usizeTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "regexSplit_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::encoding — base64Encode: (Vec<uint8>) → string ─────────
            if (calleeName == "base64Encode") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy       = getOrCreateVecStructType();
                auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, "b64enc_arg");
                builder_->CreateStore(args[0], vecArgAlloca);
                auto callee = declareBuiltin("lux_base64EncodeVec", strTy, {ptrTy});
                auto* ret = builder_->CreateCall(callee, {vecArgAlloca}, "base64Encode");
                return static_cast<llvm::Value*>(buildString(ret));
            }

            // ── std::encoding — base64Decode: (string) → Vec<uint8> ─────────
            if (calleeName == "base64Decode") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "b64dec_out");
                std::vector<llvm::Value*> callArgs = {outAlloca};
                extractStringArg(args[0], callArgs);
                auto callee = declareBuiltin("lux_base64DecodeVec", voidTy,
                    {ptrTy, ptrTy, usizeTy});
                builder_->CreateCall(callee, callArgs);
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "b64dec_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::crypto — md5/sha1/sha256/sha512: (Vec<uint8>) → string ─
            {
                static const std::unordered_map<std::string, std::string> cryptoByteFuncs = {
                    {"md5",    "lux_md5Bytes"},
                    {"sha1",   "lux_sha1Bytes"},
                    {"sha256", "lux_sha256Bytes"},
                    {"sha512", "lux_sha512Bytes"},
                };
                auto crIt = cryptoByteFuncs.find(calleeName);
                if (crIt != cryptoByteFuncs.end()) {
                    std::vector<llvm::Value*> args;
                    if (auto* argList = ctx->argList())
                        for (auto* e : argList->expression())
                            args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                    auto* vecTy       = getOrCreateVecStructType();
                    auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, calleeName + "_arg");
                    builder_->CreateStore(args[0], vecArgAlloca);
                    auto callee = declareBuiltin(crIt->second, strTy, {ptrTy});
                    auto* ret = builder_->CreateCall(callee, {vecArgAlloca}, calleeName);
                    return static_cast<llvm::Value*>(buildString(ret));
                }
            }

            // ── std::crypto — hmacSha256: (Vec<uint8>, Vec<uint8>) → Vec<uint8>
            if (calleeName == "hmacSha256") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy      = getOrCreateVecStructType();
                auto* keyAlloca  = builder_->CreateAlloca(vecTy, nullptr, "hmac_key");
                auto* dataAlloca = builder_->CreateAlloca(vecTy, nullptr, "hmac_data");
                auto* outAlloca  = builder_->CreateAlloca(vecTy, nullptr, "hmac_out");
                builder_->CreateStore(args[0], keyAlloca);
                builder_->CreateStore(args[1], dataAlloca);
                auto callee = declareBuiltin("lux_hmacSha256", voidTy,
                    {ptrTy, ptrTy, ptrTy});
                builder_->CreateCall(callee, {outAlloca, keyAlloca, dataAlloca});
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "hmac_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::crypto — randomBytes: (usize) → Vec<uint8> ────────────
            if (calleeName == "randomBytes") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy    = getOrCreateVecStructType();
                auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "randomBytes_out");
                auto* n = args[0];
                if (n->getType() != usizeTy)
                    n = builder_->CreateIntCast(n, usizeTy, false);
                auto callee = declareBuiltin("lux_randomBytes", voidTy, {ptrTy, usizeTy});
                builder_->CreateCall(callee, {outAlloca, n});
                auto* result = builder_->CreateLoad(vecTy, outAlloca, "randomBytes_val");
                return static_cast<llvm::Value*>(result);
            }

            // ── std::path — joinAll: (Vec<string>) → string ─────────────────
            if (calleeName == "joinAll") {
                std::vector<llvm::Value*> args;
                if (auto* argList = ctx->argList())
                    for (auto* e : argList->expression())
                        args.push_back(std::any_cast<llvm::Value*>(visit(e)));
                auto* vecTy       = getOrCreateVecStructType();
                auto* vecArgAlloca = builder_->CreateAlloca(vecTy, nullptr, "joinAll_arg");
                builder_->CreateStore(args[0], vecArgAlloca);
                auto callee = declareBuiltin("lux_joinAllVec", strTy, {ptrTy});
                auto* ret = builder_->CreateCall(callee, {vecArgAlloca}, "joinAll");
                return static_cast<llvm::Value*>(buildString(ret));
            }
        }

        auto it = locals_.find(calleeName);
        if (it != locals_.end() && it->second.typeInfo->kind == TypeKind::Function) {
            fnTI = it->second.typeInfo;
        } else if (!fnTI) {
            // Check module-level functions for direct call (with mangled name)
            auto resolvedName = resolveCallTarget(calleeName);
            directFn = module_->getFunction(resolvedName);

            // Lazy declare C function from #include bindings
            if (!directFn)
                directFn = declareCFunction(calleeName);
        }
    }

    // Direct call to a module-level function
    if (directFn) {
        auto* fnType = directFn->getFunctionType();

        // Check if this is a variadic function
        auto vit = variadicFunctions_.find(directFn->getName().str());
        if (vit != variadicFunctions_.end()) {
            auto& vfInfo = vit->second;
            size_t varIdx = vfInfo.variadicParamIdx;
            auto* elemTy = vfInfo.elementType->toLLVMType(*context_, module_->getDataLayout());

            std::vector<llvm::Value*> args;
            std::vector<llvm::Value*> variadicArgs;

            if (auto* argList = ctx->argList()) {
                for (size_t i = 0; i < argList->expression().size(); i++) {
                    auto* argVal = std::any_cast<llvm::Value*>(visit(argList->expression(i)));
                    if (i < varIdx) {
                        // Normal param: cast if needed
                        if (argVal->getType() != fnType->getParamType(i)) {
                            if (argVal->getType()->isIntegerTy() && fnType->getParamType(i)->isIntegerTy())
                                argVal = builder_->CreateIntCast(argVal, fnType->getParamType(i), true);
                            else if (argVal->getType()->isFloatingPointTy() && fnType->getParamType(i)->isFloatingPointTy()) {
                                if (argVal->getType()->getPrimitiveSizeInBits() > fnType->getParamType(i)->getPrimitiveSizeInBits())
                                    argVal = builder_->CreateFPTrunc(argVal, fnType->getParamType(i));
                                else
                                    argVal = builder_->CreateFPExt(argVal, fnType->getParamType(i));
                            }
                        }
                        args.push_back(argVal);
                    } else {
                        // Variadic arg: cast to element type if needed
                        if (argVal->getType() != elemTy) {
                            if (argVal->getType()->isIntegerTy() && elemTy->isIntegerTy())
                                argVal = builder_->CreateIntCast(argVal, elemTy, vfInfo.elementType->isSigned);
                            else if (argVal->getType()->isFloatingPointTy() && elemTy->isFloatingPointTy()) {
                                if (argVal->getType()->getPrimitiveSizeInBits() > elemTy->getPrimitiveSizeInBits())
                                    argVal = builder_->CreateFPTrunc(argVal, elemTy);
                                else
                                    argVal = builder_->CreateFPExt(argVal, elemTy);
                            }
                        }
                        variadicArgs.push_back(argVal);
                    }
                }
            }

            auto* i64Ty = llvm::Type::getInt64Ty(*context_);
            auto* ptrTy = llvm::PointerType::getUnqual(*context_);

            if (variadicArgs.empty()) {
                // No variadic args: pass null + 0
                args.push_back(llvm::ConstantPointerNull::get(ptrTy));
                args.push_back(llvm::ConstantInt::get(i64Ty, 0));
            } else {
                // Pack variadic args into a stack-allocated array
                auto* arrTy = llvm::ArrayType::get(elemTy, variadicArgs.size());
                auto* arrAlloca = builder_->CreateAlloca(arrTy, nullptr, "variadic.pack");
                auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                for (size_t j = 0; j < variadicArgs.size(); j++) {
                    auto* idx = llvm::ConstantInt::get(i64Ty, j);
                    auto* gep = builder_->CreateGEP(arrTy, arrAlloca, {zero, idx});
                    builder_->CreateStore(variadicArgs[j], gep);
                }
                args.push_back(arrAlloca);
                args.push_back(llvm::ConstantInt::get(i64Ty, variadicArgs.size()));
            }

            auto* result = builder_->CreateCall(directFn, args,
                directFn->getReturnType()->isVoidTy() ? "" : "call");
            return static_cast<llvm::Value*>(result);
        }

        // ── C extern variadic function as expression (e.g. printf) ──
        if (externCFunctions_.count(directFn->getName().str()) && directFn->isVarArg()) {
            unsigned fixedParams = fnType->getNumParams();
            std::vector<llvm::Value*> args;
            if (auto* argList = ctx->argList()) {
                for (size_t i = 0; i < argList->expression().size(); i++) {
                    auto* argVal = std::any_cast<llvm::Value*>(visit(argList->expression(i)));
                    if (i < fixedParams) {
                        if (argVal->getType() != fnType->getParamType(i)) {
                            if (argVal->getType()->isIntegerTy() && fnType->getParamType(i)->isIntegerTy())
                                argVal = builder_->CreateIntCast(argVal, fnType->getParamType(i), true);
                            else if (argVal->getType()->isFloatingPointTy() && fnType->getParamType(i)->isFloatingPointTy())
                                argVal = builder_->CreateFPExt(argVal, fnType->getParamType(i));
                        }
                    } else {
                        // C default argument promotions
                        if (argVal->getType()->isFloatTy())
                            argVal = builder_->CreateFPExt(argVal, llvm::Type::getDoubleTy(*context_), "c_promote_f64");
                        else if (argVal->getType()->isIntegerTy()) {
                            unsigned bw = argVal->getType()->getIntegerBitWidth();
                            if (bw < 32)
                                argVal = builder_->CreateSExt(argVal, llvm::Type::getInt32Ty(*context_), "c_promote_i32");
                            else if (bw > 64)
                                argVal = builder_->CreateTrunc(argVal, llvm::Type::getInt32Ty(*context_), "c_trunc_i32");
                        }
                    }
                    args.push_back(argVal);
                }
            }
            auto* result = builder_->CreateCall(directFn, args,
                directFn->getReturnType()->isVoidTy() ? "" : "call");
            return static_cast<llvm::Value*>(result);
        }

        std::vector<llvm::Value*> args;

        // Check if this C function needs ABI struct coercion
        auto abiIt = cabiInfos_.find(directFn->getName().str());
        bool needsABI = (abiIt != cabiInfos_.end());
        CABIInfo* abi = needsABI ? &abiIt->second : nullptr;

        // Sret: allocate space for large struct return, pass as hidden first arg
        llvm::AllocaInst* sretAlloca = nullptr;
        if (abi && abi->returnInfo.kind == ABIArgKind::Indirect) {
            sretAlloca = builder_->CreateAlloca(abi->originalRetType, nullptr, "sret");
            args.push_back(sretAlloca);
        }

        if (auto* argList = ctx->argList()) {
            for (size_t i = 0; i < argList->expression().size(); i++) {
                auto* argVal = std::any_cast<llvm::Value*>(visit(argList->expression(i)));

                // ABI struct coercion for C functions
                if (abi && i < abi->paramInfos.size()) {
                    auto& pi = abi->paramInfos[i];
                    if (pi.kind == ABIArgKind::Coerce && argVal->getType()->isStructTy()) {
                        // Store struct → load as coerced integer type
                        auto* tmp = builder_->CreateAlloca(argVal->getType(), nullptr, "abi.coerce");
                        builder_->CreateStore(argVal, tmp);
                        argVal = builder_->CreateLoad(pi.coercedType, tmp, "abi.coerced");
                        args.push_back(argVal);
                        continue;
                    }
                    if (pi.kind == ABIArgKind::Expand && argVal->getType()->isStructTy()) {
                        // Store struct → load as split parts (e.g., two i64 args)
                        auto* tmp = builder_->CreateAlloca(argVal->getType(), nullptr, "abi.expand");
                        builder_->CreateStore(argVal, tmp);
                        auto* i8Ty = llvm::Type::getInt8Ty(*context_);
                        uint64_t offset = 0;
                        for (auto* partTy : pi.expandedTypes) {
                            llvm::Value* partPtr = tmp;
                            if (offset > 0)
                                partPtr = builder_->CreateGEP(i8Ty, tmp,
                                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), offset));
                            args.push_back(builder_->CreateLoad(partTy, partPtr, "abi.part"));
                            offset += module_->getDataLayout().getTypeAllocSize(partTy);
                        }
                        continue;
                    }
                    if (pi.kind == ABIArgKind::Indirect && argVal->getType()->isStructTy()) {
                        // Store struct → pass pointer (byval)
                        auto* tmp = builder_->CreateAlloca(argVal->getType(), nullptr, "abi.byval");
                        builder_->CreateStore(argVal, tmp);
                        args.push_back(tmp);
                        continue;
                    }
                }

                // Standard type coercion (non-struct)
                if (i < fnType->getNumParams() && argVal->getType() != fnType->getParamType(i)) {
                    // Compute actual param index (accounting for sret offset)
                    unsigned llvmIdx = static_cast<unsigned>(i);
                    if (sretAlloca) llvmIdx++;
                    if (llvmIdx < fnType->getNumParams()) {
                        auto* expectedTy = fnType->getParamType(llvmIdx);
                        if (argVal->getType()->isArrayTy() && expectedTy->isPointerTy()) {
                            auto* tmpAlloca = builder_->CreateAlloca(argVal->getType(), nullptr, "arr.decay");
                            builder_->CreateStore(argVal, tmpAlloca);
                            auto* zero = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0);
                            argVal = builder_->CreateGEP(argVal->getType(), tmpAlloca, {zero, zero}, "arr.ptr");
                        } else if (argVal->getType()->isIntegerTy() && expectedTy->isIntegerTy())
                            argVal = builder_->CreateIntCast(argVal, expectedTy, true);
                        else if (argVal->getType()->isFloatingPointTy() && expectedTy->isFloatingPointTy()) {
                            if (argVal->getType()->getPrimitiveSizeInBits() > expectedTy->getPrimitiveSizeInBits())
                                argVal = builder_->CreateFPTrunc(argVal, expectedTy);
                            else
                                argVal = builder_->CreateFPExt(argVal, expectedTy);
                        }
                    }
                }
                args.push_back(argVal);
            }
        }

        auto* callResult = builder_->CreateCall(directFn, args,
            directFn->getReturnType()->isVoidTy() ? "" : "call");

        // ABI return coercion: convert coerced return back to original struct
        if (abi) {
            if (abi->returnInfo.kind == ABIArgKind::Indirect) {
                // Sret: result is in the alloca
                auto* structVal = builder_->CreateLoad(abi->originalRetType, sretAlloca, "sret.load");
                return static_cast<llvm::Value*>(structVal);
            }
            if (abi->returnInfo.kind == ABIArgKind::Coerce && abi->originalRetType->isStructTy()) {
                // Coerced return (e.g., i64 → {i32, i32})
                auto* tmp = builder_->CreateAlloca(abi->returnInfo.coercedType, nullptr, "abi.ret");
                builder_->CreateStore(callResult, tmp);
                auto* structVal = builder_->CreateLoad(abi->originalRetType, tmp, "abi.uncoerce");
                return static_cast<llvm::Value*>(structVal);
            }
        }

        return static_cast<llvm::Value*>(callResult);
    }

    // Indirect call via function pointer variable
    if (!fnTI || fnTI->kind != TypeKind::Function) {
        std::cerr << "lux: expression is not a function type\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    // Get the function pointer value
    auto* fnPtr = std::any_cast<llvm::Value*>(visit(baseExpr));

    // Build LLVM FunctionType from TypeInfo
    auto* retLLVM = fnTI->returnType->toLLVMType(*context_, module_->getDataLayout());
    std::vector<llvm::Type*> paramLLVM;
    for (auto* pti : fnTI->paramTypes) {
        paramLLVM.push_back(pti->toLLVMType(*context_, module_->getDataLayout()));
    }
    auto* fnType = llvm::FunctionType::get(retLLVM, paramLLVM, false);

    // Collect arguments
    std::vector<llvm::Value*> args;
    if (auto* argList = ctx->argList()) {
        for (size_t i = 0; i < argList->expression().size(); i++) {
            auto* argVal = std::any_cast<llvm::Value*>(visit(argList->expression(i)));
            // Cast if needed
            if (i < paramLLVM.size() && argVal->getType() != paramLLVM[i]) {
                if (argVal->getType()->isIntegerTy() && paramLLVM[i]->isIntegerTy())
                    argVal = builder_->CreateIntCast(argVal, paramLLVM[i],
                                                     fnTI->paramTypes[i]->isSigned);
                else if (argVal->getType()->isFloatingPointTy() && paramLLVM[i]->isFloatingPointTy()) {
                    if (argVal->getType()->getPrimitiveSizeInBits() > paramLLVM[i]->getPrimitiveSizeInBits())
                        argVal = builder_->CreateFPTrunc(argVal, paramLLVM[i]);
                    else
                        argVal = builder_->CreateFPExt(argVal, paramLLVM[i]);
                }
            }
            args.push_back(argVal);
        }
    }

    auto* result = builder_->CreateCall(
        llvm::FunctionCallee(fnType, fnPtr), args, "fncall");
    return static_cast<llvm::Value*>(result);
}

std::any IRGen::visitNegExpr(LuxParser::NegExprContext* ctx) {
    auto* val = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    if (val->getType()->isFloatingPointTy())
        return static_cast<llvm::Value*>(builder_->CreateFNeg(val, "neg"));
    return static_cast<llvm::Value*>(builder_->CreateNeg(val, "neg"));
}

std::any IRGen::visitMulExpr(LuxParser::MulExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto [l, r] = promoteArithmetic(lhs, rhs);
    bool isFloat = l->getType()->isFloatingPointTy();

    // Division by zero check for integer / and %
    if (!isFloat && (ctx->op->getType() == LuxLexer::SLASH ||
                     ctx->op->getType() == LuxLexer::PERCENT)) {
        auto* zero    = llvm::ConstantInt::get(r->getType(), 0);
        auto* isZero  = builder_->CreateICmpEQ(r, zero, "div_zero_chk");

        auto* fn      = currentFunction_;
        auto* throwBB = llvm::BasicBlock::Create(*context_, "div.throw", fn);
        auto* okBB    = llvm::BasicBlock::Create(*context_, "div.ok",    fn);

        builder_->CreateCondBr(isZero, throwBB, okBB);

        // Emit throw for division by zero
        builder_->SetInsertPoint(throwBB);
        {
            auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
            auto* voidTy  = llvm::Type::getVoidTy(*context_);
            auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
            auto* i32Ty   = llvm::Type::getInt32Ty(*context_);
            auto* errorTy = llvm::StructType::getTypeByName(*context_, "Error");
            auto* strTy   = llvm::StructType::get(*context_, { ptrTy, usizeTy });

            // Build message string
            auto* msgGlobal = builder_->CreateGlobalString(
                "division by zero", ".div_err_msg", 0, module_);
            auto* msgLen = llvm::ConstantInt::get(usizeTy, 17);
            llvm::Value* msgStr = llvm::UndefValue::get(strTy);
            msgStr = builder_->CreateInsertValue(msgStr, msgGlobal, {0});
            msgStr = builder_->CreateInsertValue(msgStr, msgLen,    {1});

            // Build file string
            std::string fileName = module_->getSourceFileName();
            auto* fileGlobal = builder_->CreateGlobalString(
                fileName, ".div_err_file", 0, module_);
            auto* fileLen = llvm::ConstantInt::get(usizeTy, fileName.size());
            llvm::Value* fileStr = llvm::UndefValue::get(strTy);
            fileStr = builder_->CreateInsertValue(fileStr, fileGlobal, {0});
            fileStr = builder_->CreateInsertValue(fileStr, fileLen,    {1});

            auto* opToken = ctx->op;
            auto* line = llvm::ConstantInt::get(i32Ty, opToken->getLine());
            auto* col  = llvm::ConstantInt::get(i32Ty,
                             opToken->getCharPositionInLine());

            // Build Error struct
            llvm::Value* errVal = llvm::ConstantAggregateZero::get(errorTy);
            errVal = builder_->CreateInsertValue(errVal, msgStr,  {0});
            errVal = builder_->CreateInsertValue(errVal, fileStr, {1});
            errVal = builder_->CreateInsertValue(errVal, line,    {2});
            errVal = builder_->CreateInsertValue(errVal, col,     {3});

            auto* errAlloca = builder_->CreateAlloca(errorTy, nullptr, "div_err");
            builder_->CreateStore(errVal, errAlloca);

            auto throwFn = declareBuiltin("lux_eh_throw", voidTy, {ptrTy});
            builder_->CreateCall(throwFn, {errAlloca});
            builder_->CreateUnreachable();
        }

        builder_->SetInsertPoint(okBB);
    }

    switch (ctx->op->getType()) {
    case LuxLexer::STAR:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFMul(l, r, "mul")
                    : builder_->CreateMul(l, r, "mul"));
    case LuxLexer::SLASH:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFDiv(l, r, "div")
                    : builder_->CreateSDiv(l, r, "div"));
    case LuxLexer::PERCENT:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFRem(l, r, "mod")
                    : builder_->CreateSRem(l, r, "mod"));
    default:
        return static_cast<llvm::Value*>(llvm::UndefValue::get(l->getType()));
    }
}

std::any IRGen::visitAddSubExpr(LuxParser::AddSubExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));

    bool lhsPtr = lhs->getType()->isPointerTy();
    bool rhsPtr = rhs->getType()->isPointerTy();

    // ── Pointer arithmetic ──────────────────────────────────────────
    if (lhsPtr || rhsPtr) {
        auto* i8Ty = llvm::Type::getInt8Ty(*context_);

        // Resolve the pointee LLVM type for proper stride calculation
        auto getPointeeTy = [&](LuxParser::ExpressionContext* e) -> llvm::Type* {
            auto* ti = resolveExprTypeInfo(e);
            if (ti && ti->kind == TypeKind::Pointer && ti->pointeeType) {
                return ti->pointeeType->toLLVMType(*context_, module_->getDataLayout());
            }
            return i8Ty; // fallback: byte-level arithmetic
        };

        if (lhsPtr && !rhsPtr) {
            // ptr +/- int
            auto* elemTy = getPointeeTy(ctx->expression(0));
            auto* idx = rhs;
            if (ctx->op->getType() == LuxLexer::MINUS) {
                idx = builder_->CreateNeg(rhs, "neg");
            }
            return static_cast<llvm::Value*>(
                builder_->CreateGEP(elemTy, lhs, idx, "ptradd"));
        }
        if (!lhsPtr && rhsPtr) {
            // int + ptr
            auto* elemTy = getPointeeTy(ctx->expression(1));
            return static_cast<llvm::Value*>(
                builder_->CreateGEP(elemTy, rhs, lhs, "ptradd"));
        }
        // ptr - ptr → ptrdiff (i64)
        auto* elemTy = getPointeeTy(ctx->expression(0));
        auto* i64Ty = llvm::Type::getInt64Ty(*context_);
        auto* lhsInt = builder_->CreatePtrToInt(lhs, i64Ty, "ptr2int");
        auto* rhsInt = builder_->CreatePtrToInt(rhs, i64Ty, "ptr2int");
        auto* diff = builder_->CreateSub(lhsInt, rhsInt, "ptrdiff");
        auto  elemSize = module_->getDataLayout().getTypeAllocSize(elemTy);
        if (elemSize > 1) {
            diff = builder_->CreateExactSDiv(diff,
                llvm::ConstantInt::get(i64Ty, elemSize), "ptrdiff.elems");
        }
        return static_cast<llvm::Value*>(diff);
    }

    // ── Normal arithmetic ───────────────────────────────────────────
    auto [l, r] = promoteArithmetic(lhs, rhs);
    bool isFloat = l->getType()->isFloatingPointTy();

    switch (ctx->op->getType()) {
    case LuxLexer::PLUS:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFAdd(l, r, "add")
                    : builder_->CreateAdd(l, r, "add"));
    case LuxLexer::MINUS:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFSub(l, r, "sub")
                    : builder_->CreateSub(l, r, "sub"));
    default:
        return static_cast<llvm::Value*>(llvm::UndefValue::get(l->getType()));
    }
}

std::any IRGen::visitLogicalNotExpr(LuxParser::LogicalNotExprContext* ctx) {
    auto* val = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    // Truncate to i1 if not already bool
    if (!val->getType()->isIntegerTy(1))
        val = builder_->CreateICmpNE(val,
            llvm::ConstantInt::get(val->getType(), 0), "tobool");
    return static_cast<llvm::Value*>(builder_->CreateNot(val, "not"));
}

std::any IRGen::visitLogicalAndExpr(LuxParser::LogicalAndExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    // Convert to i1 if needed
    if (!lhs->getType()->isIntegerTy(1))
        lhs = builder_->CreateICmpNE(lhs,
            llvm::ConstantInt::get(lhs->getType(), 0), "tobool");
    if (!rhs->getType()->isIntegerTy(1))
        rhs = builder_->CreateICmpNE(rhs,
            llvm::ConstantInt::get(rhs->getType(), 0), "tobool");
    return static_cast<llvm::Value*>(builder_->CreateAnd(lhs, rhs, "and"));
}

std::any IRGen::visitLogicalOrExpr(LuxParser::LogicalOrExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    if (!lhs->getType()->isIntegerTy(1))
        lhs = builder_->CreateICmpNE(lhs,
            llvm::ConstantInt::get(lhs->getType(), 0), "tobool");
    if (!rhs->getType()->isIntegerTy(1))
        rhs = builder_->CreateICmpNE(rhs,
            llvm::ConstantInt::get(rhs->getType(), 0), "tobool");
    return static_cast<llvm::Value*>(builder_->CreateOr(lhs, rhs, "or"));
}

std::any IRGen::visitBitNotExpr(LuxParser::BitNotExprContext* ctx) {
    auto* val = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    return static_cast<llvm::Value*>(builder_->CreateNot(val, "bitnot"));
}

std::any IRGen::visitShiftExpr(LuxParser::ShiftExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto [l, r] = promoteArithmetic(lhs, rhs);

    switch (ctx->op->getType()) {
    case LuxLexer::LSHIFT:
        return static_cast<llvm::Value*>(builder_->CreateShl(l, r, "shl"));
    case LuxLexer::RSHIFT:
        return static_cast<llvm::Value*>(builder_->CreateAShr(l, r, "shr"));
    default:
        return static_cast<llvm::Value*>(llvm::UndefValue::get(l->getType()));
    }
}

std::any IRGen::visitBitAndExpr(LuxParser::BitAndExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto [l, r] = promoteArithmetic(lhs, rhs);
    return static_cast<llvm::Value*>(builder_->CreateAnd(l, r, "bitand"));
}

std::any IRGen::visitBitXorExpr(LuxParser::BitXorExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto [l, r] = promoteArithmetic(lhs, rhs);
    return static_cast<llvm::Value*>(builder_->CreateXor(l, r, "bitxor"));
}

std::any IRGen::visitBitOrExpr(LuxParser::BitOrExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto [l, r] = promoteArithmetic(lhs, rhs);
    return static_cast<llvm::Value*>(builder_->CreateOr(l, r, "bitor"));
}

std::any IRGen::visitPreIncrExpr(LuxParser::PreIncrExprContext* ctx) {
    auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(ctx->expression());
    if (!identBase) {
        std::cerr << "lux: ++/-- requires a variable\n";
        return static_cast<llvm::Value*>(llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto name = identBase->IDENTIFIER()->getText();
    auto it = locals_.find(name);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << name << "'\n";
        return static_cast<llvm::Value*>(llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto* alloca = it->second.alloca;
    auto* varTy = alloca->getAllocatedType();
    auto* cur = builder_->CreateLoad(varTy, alloca, name);
    auto* one = llvm::ConstantInt::get(varTy, 1);
    auto* inc = builder_->CreateAdd(cur, one, "preinc");
    builder_->CreateStore(inc, alloca);
    return static_cast<llvm::Value*>(inc);
}

std::any IRGen::visitPreDecrExpr(LuxParser::PreDecrExprContext* ctx) {
    auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(ctx->expression());
    if (!identBase) {
        std::cerr << "lux: ++/-- requires a variable\n";
        return static_cast<llvm::Value*>(llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto name = identBase->IDENTIFIER()->getText();
    auto it = locals_.find(name);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << name << "'\n";
        return static_cast<llvm::Value*>(llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto* alloca = it->second.alloca;
    auto* varTy = alloca->getAllocatedType();
    auto* cur = builder_->CreateLoad(varTy, alloca, name);
    auto* one = llvm::ConstantInt::get(varTy, 1);
    auto* dec = builder_->CreateSub(cur, one, "predec");
    builder_->CreateStore(dec, alloca);
    return static_cast<llvm::Value*>(dec);
}

std::any IRGen::visitPostIncrExpr(LuxParser::PostIncrExprContext* ctx) {
    auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(ctx->expression());
    if (!identBase) {
        std::cerr << "lux: ++/-- requires a variable\n";
        return static_cast<llvm::Value*>(llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto name = identBase->IDENTIFIER()->getText();
    auto it = locals_.find(name);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << name << "'\n";
        return static_cast<llvm::Value*>(llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto* alloca = it->second.alloca;
    auto* varTy = alloca->getAllocatedType();
    auto* old = builder_->CreateLoad(varTy, alloca, name);
    auto* one = llvm::ConstantInt::get(varTy, 1);
    auto* inc = builder_->CreateAdd(old, one, "postinc");
    builder_->CreateStore(inc, alloca);
    return static_cast<llvm::Value*>(old);
}

std::any IRGen::visitPostDecrExpr(LuxParser::PostDecrExprContext* ctx) {
    auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(ctx->expression());
    if (!identBase) {
        std::cerr << "lux: ++/-- requires a variable\n";
        return static_cast<llvm::Value*>(llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto name = identBase->IDENTIFIER()->getText();
    auto it = locals_.find(name);
    if (it == locals_.end()) {
        std::cerr << "lux: undefined variable '" << name << "'\n";
        return static_cast<llvm::Value*>(llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }
    auto* alloca = it->second.alloca;
    auto* varTy = alloca->getAllocatedType();
    auto* old = builder_->CreateLoad(varTy, alloca, name);
    auto* one = llvm::ConstantInt::get(varTy, 1);
    auto* dec = builder_->CreateSub(old, one, "postdec");
    builder_->CreateStore(dec, alloca);
    return static_cast<llvm::Value*>(old);
}

std::any IRGen::visitCastExpr(LuxParser::CastExprContext* ctx) {
    auto* val = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    auto* ti = resolveTypeInfo(ctx->typeSpec());
    auto* targetTy = ti->toLLVMType(*context_, module_->getDataLayout());
    auto* srcTy = val->getType();

    if (srcTy == targetTy)
        return static_cast<llvm::Value*>(val);

    // int -> int
    if (srcTy->isIntegerTy() && targetTy->isIntegerTy()) {
        return static_cast<llvm::Value*>(
            builder_->CreateIntCast(val, targetTy, ti->isSigned, "cast"));
    }
    // float -> float
    if (srcTy->isFloatingPointTy() && targetTy->isFloatingPointTy()) {
        if (srcTy->getPrimitiveSizeInBits() > targetTy->getPrimitiveSizeInBits())
            return static_cast<llvm::Value*>(
                builder_->CreateFPTrunc(val, targetTy, "cast"));
        else
            return static_cast<llvm::Value*>(
                builder_->CreateFPExt(val, targetTy, "cast"));
    }
    // int -> float
    if (srcTy->isIntegerTy() && targetTy->isFloatingPointTy()) {
        return static_cast<llvm::Value*>(
            ti->isSigned ? builder_->CreateSIToFP(val, targetTy, "cast")
                         : builder_->CreateUIToFP(val, targetTy, "cast"));
    }
    // float -> int
    if (srcTy->isFloatingPointTy() && targetTy->isIntegerTy()) {
        return static_cast<llvm::Value*>(
            ti->isSigned ? builder_->CreateFPToSI(val, targetTy, "cast")
                         : builder_->CreateFPToUI(val, targetTy, "cast"));
    }
    // pointer -> pointer (bitcast is a no-op with opaque pointers)
    if (srcTy->isPointerTy() && targetTy->isPointerTy())
        return static_cast<llvm::Value*>(val);
    // int -> pointer
    if (srcTy->isIntegerTy() && targetTy->isPointerTy())
        return static_cast<llvm::Value*>(
            builder_->CreateIntToPtr(val, targetTy, "cast"));
    // pointer -> int
    if (srcTy->isPointerTy() && targetTy->isIntegerTy())
        return static_cast<llvm::Value*>(
            builder_->CreatePtrToInt(val, targetTy, "cast"));

    std::cerr << "lux: unsupported cast\n";
    return static_cast<llvm::Value*>(val);
}

std::any IRGen::visitSizeofExpr(LuxParser::SizeofExprContext* ctx) {
    auto* ti = resolveTypeInfo(ctx->typeSpec());
    auto* llvmTy = ti->toLLVMType(*context_, module_->getDataLayout());
    auto& dl = module_->getDataLayout();
    uint64_t sizeBytes = dl.getTypeAllocSize(llvmTy);
    return static_cast<llvm::Value*>(
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), sizeBytes));
}

std::any IRGen::visitTypeofExpr(LuxParser::TypeofExprContext* ctx) {
    // Resolve expression type at compile-time
    auto* exprVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    // Try to find TypeInfo for the expression
    std::string typeName = "unknown";

    if (auto* load = llvm::dyn_cast<llvm::LoadInst>(exprVal)) {
        auto* src = load->getPointerOperand();
        for (auto& [vname, info] : locals_) {
            if (info.alloca == src) {
                typeName = info.typeInfo->name;
                break;
            }
        }
        // Struct field lookup via GEP
        if (typeName == "unknown") {
            if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(src)) {
                auto* base = gep->getPointerOperand();
                for (auto& [vname, info] : locals_) {
                    if (info.alloca == base &&
                        info.typeInfo && info.typeInfo->kind == TypeKind::Struct) {
                        if (gep->getNumIndices() == 2) {
                            if (auto* ci = llvm::dyn_cast<llvm::ConstantInt>(
                                    gep->getOperand(2))) {
                                unsigned idx = ci->getZExtValue();
                                if (idx < info.typeInfo->fields.size())
                                    typeName = info.typeInfo->fields[idx].typeInfo->name;
                            }
                        }
                        break;
                    }
                }
            }
        }
    } else {
        // Infer from LLVM type for literals/constants
        auto* ty = exprVal->getType();
        if (ty->isIntegerTy(1))       typeName = "bool";
        else if (ty->isIntegerTy(8))  typeName = "int8";
        else if (ty->isIntegerTy(16)) typeName = "int16";
        else if (ty->isIntegerTy(32)) typeName = "int32";
        else if (ty->isIntegerTy(64)) typeName = "int64";
        else if (ty->isFloatTy())     typeName = "float32";
        else if (ty->isDoubleTy())    typeName = "float64";
        else if (ty->isStructTy())    typeName = "string";
        else if (ty->isPointerTy())   typeName = "pointer";
    }

    // AST-based fallback for method calls and function calls
    if (typeName == "unknown") {
        auto* exprCtx = ctx->expression();

        if (auto* mc = dynamic_cast<LuxParser::MethodCallExprContext*>(exprCtx)) {
            auto* receiverTI = resolveExprTypeInfo(mc->expression());
            if (receiverTI) {
                std::string mname = mc->IDENTIFIER()->getText();

                if (receiverTI->kind == TypeKind::Extended) {
                    // Map.keys() → Vec<K>, Map.values() → Vec<V>
                    if (receiverTI->extendedKind == "Map") {
                        if (mname == "keys" && receiverTI->keyType)
                            typeName = "Vec<" + receiverTI->keyType->name + ">";
                        else if (mname == "values" && receiverTI->valueType)
                            typeName = "Vec<" + receiverTI->valueType->name + ">";
                    }
                    // Set.values() → Vec<T>
                    else if (receiverTI->extendedKind == "Set") {
                        if (mname == "values" && receiverTI->elementType)
                            typeName = "Vec<" + receiverTI->elementType->name + ">";
                    }

                    // Generic Extended type method lookup
                    if (typeName == "unknown") {
                        auto* desc = extTypeRegistry_.lookup(receiverTI->extendedKind);
                        if (desc) {
                            for (auto& m : desc->methods) {
                                if (m.name == mname) {
                                    if (m.returnType == "_self")
                                        typeName = receiverTI->name;
                                    else
                                        typeName = m.returnType;
                                    break;
                                }
                            }
                        }
                    }
                }
                // String / Integer / Float method lookup
                else if (receiverTI->kind == TypeKind::String ||
                         receiverTI->kind == TypeKind::Integer ||
                         receiverTI->kind == TypeKind::Float) {
                    auto* md = methodRegistry_.lookup(receiverTI->kind,
                                                      mc->IDENTIFIER()->getText());
                    if (md) {
                        typeName = (md->returnType == "_self")
                                       ? receiverTI->name
                                       : md->returnType;
                    }
                }
            }
        }
    }

    // Emit as string constant (fat pointer: {ptr, len})
    auto* strGlobal = builder_->CreateGlobalString(typeName, ".typeof", 0, module_);
    auto* strTy   = typeRegistry_.lookup("string")
                        ->toLLVMType(*context_, module_->getDataLayout());
    auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
    auto* len     = llvm::ConstantInt::get(usizeTy, typeName.size());

    llvm::Value* strStruct = llvm::UndefValue::get(strTy);
    strStruct = builder_->CreateInsertValue(strStruct, strGlobal, 0, "str_ptr");
    strStruct = builder_->CreateInsertValue(strStruct, len,       1, "str_len");
    return static_cast<llvm::Value*>(strStruct);
}

std::any IRGen::visitTernaryExpr(LuxParser::TernaryExprContext* ctx) {
    auto* cond = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    // Convert to i1 if not already bool
    if (!cond->getType()->isIntegerTy(1))
        cond = builder_->CreateICmpNE(cond,
            llvm::ConstantInt::get(cond->getType(), 0), "tobool");

    auto* trueVal  = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto* falseVal = std::any_cast<llvm::Value*>(visit(ctx->expression(2)));

    // Promote to common type
    auto [t, f] = promoteArithmetic(trueVal, falseVal);
    return static_cast<llvm::Value*>(builder_->CreateSelect(cond, t, f, "ternary"));
}

std::any IRGen::visitIsExpr(LuxParser::IsExprContext* ctx) {
    // Static type check — resolved at compile time
    auto* val = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    auto* targetTI = resolveTypeInfo(ctx->typeSpec());
    auto* targetTy = targetTI->toLLVMType(*context_, module_->getDataLayout());

    // Compare the LLVM types at compile time
    bool match = (val->getType() == targetTy);
    return static_cast<llvm::Value*>(
        llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), match ? 1 : 0));
}

std::any IRGen::visitNullCoalExpr(LuxParser::NullCoalExprContext* ctx) {
    auto* ptrVal = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));

    // Check if pointer is null
    auto* isNull = builder_->CreateICmpEQ(
        ptrVal,
        llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(*context_)),
        "isnull");

    auto* thenBB  = llvm::BasicBlock::Create(*context_, "coal.nonnull", currentFunction_);
    auto* elseBB  = llvm::BasicBlock::Create(*context_, "coal.null", currentFunction_);
    auto* mergeBB = llvm::BasicBlock::Create(*context_, "coal.end", currentFunction_);

    builder_->CreateCondBr(isNull, elseBB, thenBB);

    // Non-null path: dereference the pointer to get the value
    builder_->SetInsertPoint(thenBB);
    auto* defaultVal = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto* loadedVal = builder_->CreateLoad(defaultVal->getType(), ptrVal, "coal.load");
    builder_->CreateBr(mergeBB);
    auto* thenEnd = builder_->GetInsertBlock();

    // Null path: use the default value
    builder_->SetInsertPoint(elseBB);
    auto* fallback = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    builder_->CreateBr(mergeBB);
    auto* elseEnd = builder_->GetInsertBlock();

    // Merge
    builder_->SetInsertPoint(mergeBB);
    auto* phi = builder_->CreatePHI(defaultVal->getType(), 2, "coal.result");
    phi->addIncoming(loadedVal, thenEnd);
    phi->addIncoming(fallback, elseEnd);

    return static_cast<llvm::Value*>(phi);
}

std::any IRGen::visitRangeExpr(LuxParser::RangeExprContext* ctx) {
    // Range creates a {start, end} struct
    auto* startVal = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* endVal   = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));

    auto* elemTy = startVal->getType();
    auto* rangeTy = llvm::StructType::get(*context_, {elemTy, elemTy});

    llvm::Value* rangeVal = llvm::UndefValue::get(rangeTy);
    rangeVal = builder_->CreateInsertValue(rangeVal, startVal, 0, "range.start");
    rangeVal = builder_->CreateInsertValue(rangeVal, endVal,   1, "range.end");
    return static_cast<llvm::Value*>(rangeVal);
}

std::any IRGen::visitRangeInclExpr(LuxParser::RangeInclExprContext* ctx) {
    // Inclusive range: same struct as range, semantics differ at use site (for loop)
    auto* startVal = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* endVal   = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));

    auto* elemTy = startVal->getType();
    auto* rangeTy = llvm::StructType::get(*context_, {elemTy, elemTy});

    llvm::Value* rangeVal = llvm::UndefValue::get(rangeTy);
    rangeVal = builder_->CreateInsertValue(rangeVal, startVal, 0, "range.start");
    rangeVal = builder_->CreateInsertValue(rangeVal, endVal,   1, "range.end");
    return static_cast<llvm::Value*>(rangeVal);
}

std::any IRGen::visitSpreadExpr(LuxParser::SpreadExprContext* ctx) {
    // Spread just evaluates the inner expression — expansion handled at call/array site
    return visit(ctx->expression());
}

std::any IRGen::visitParenExpr(LuxParser::ParenExprContext* ctx) {
    return visit(ctx->expression());
}

std::any IRGen::visitRelExpr(LuxParser::RelExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto [l, r] = promoteArithmetic(lhs, rhs);
    bool isFloat = l->getType()->isFloatingPointTy();

    switch (ctx->op->getType()) {
    case LuxLexer::LT:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFCmpOLT(l, r, "lt")
                    : builder_->CreateICmpSLT(l, r, "lt"));
    case LuxLexer::GT:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFCmpOGT(l, r, "gt")
                    : builder_->CreateICmpSGT(l, r, "gt"));
    case LuxLexer::LTE:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFCmpOLE(l, r, "lte")
                    : builder_->CreateICmpSLE(l, r, "lte"));
    case LuxLexer::GTE:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFCmpOGE(l, r, "gte")
                    : builder_->CreateICmpSGE(l, r, "gte"));
    default:
        return static_cast<llvm::Value*>(llvm::UndefValue::get(l->getType()));
    }
}

std::any IRGen::visitEqExpr(LuxParser::EqExprContext* ctx) {
    auto* lhs = std::any_cast<llvm::Value*>(visit(ctx->expression(0)));
    auto* rhs = std::any_cast<llvm::Value*>(visit(ctx->expression(1)));
    auto [l, r] = promoteArithmetic(lhs, rhs);
    bool isFloat = l->getType()->isFloatingPointTy();

    switch (ctx->op->getType()) {
    case LuxLexer::EQ:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFCmpOEQ(l, r, "eq")
                    : builder_->CreateICmpEQ(l, r, "eq"));
    case LuxLexer::NEQ:
        return static_cast<llvm::Value*>(
            isFloat ? builder_->CreateFCmpONE(l, r, "neq")
                    : builder_->CreateICmpNE(l, r, "neq"));
    default:
        return static_cast<llvm::Value*>(llvm::UndefValue::get(l->getType()));
    }
}

// ── Helpers ─────────────────────────────────────────────────────────────────

const TypeInfo* IRGen::resolveTypeInfo(LuxParser::TypeSpecContext* ctx) {
    // Check for pointer type: *T
    if (isPointerType(ctx)) {
        auto* inner = ctx->typeSpec(0);
        auto* pointeeTI = resolveTypeInfo(inner);
        auto ptrName = "*" + pointeeTI->name;

        // Check if pointer type already registered
        if (auto* existing = typeRegistry_.lookup(ptrName))
            return existing;

        // Create and register pointer type
        TypeInfo ti;
        ti.name = ptrName;
        ti.kind = TypeKind::Pointer;
        ti.bitWidth = 0;
        ti.isSigned = false;
        ti.builtinSuffix = "ptr";
        ti.pointeeType = pointeeTI;
        typeRegistry_.registerType(std::move(ti));
        return typeRegistry_.lookup(ptrName);
    }

    // Check for inline function type: fn(T1, T2) -> T3
    if (auto* fnSpec = ctx->fnTypeSpec()) {
        auto specs = fnSpec->typeSpec();
        auto* retTI = resolveTypeInfo(specs.back());

        // Build a canonical name for the function type
        std::string fnName = "fn(";
        for (size_t i = 0; i + 1 < specs.size(); i++) {
            if (i > 0) fnName += ", ";
            fnName += resolveTypeInfo(specs[i])->name;
        }
        fnName += ") -> " + retTI->name;

        if (auto* existing = typeRegistry_.lookup(fnName))
            return existing;

        TypeInfo ti;
        ti.name = fnName;
        ti.kind = TypeKind::Function;
        ti.bitWidth = 0;
        ti.isSigned = false;
        ti.builtinSuffix = "ptr";
        ti.returnType = retTI;
        for (size_t i = 0; i + 1 < specs.size(); i++) {
            ti.paramTypes.push_back(resolveTypeInfo(specs[i]));
        }
        typeRegistry_.registerType(std::move(ti));
        return typeRegistry_.lookup(fnName);
    }

    // Generic extended type: Vec<int32>, Map<string, int64>
    if (ctx->LT()) {
        auto baseName = ctx->IDENTIFIER()->getText();
        auto typeParams = ctx->typeSpec();

        if (typeParams.size() == 1) {
            // Single type param: Vec<T>
            auto* elemTI = resolveTypeInfo(typeParams[0]);
            auto fullName = baseName + "<" + elemTI->name + ">";
            if (auto* existing = typeRegistry_.lookup(fullName))
                return existing;

            TypeInfo ti;
            ti.name = fullName;
            ti.kind = TypeKind::Extended;
            ti.bitWidth = 0;
            ti.isSigned = false;
            ti.builtinSuffix = elemTI->builtinSuffix;
            ti.elementType = elemTI;
            ti.extendedKind = baseName;
            typeRegistry_.registerType(std::move(ti));
            return typeRegistry_.lookup(fullName);
        } else if (typeParams.size() == 2) {
            // Two type params: Map<K, V>
            auto* keyTI = resolveTypeInfo(typeParams[0]);
            auto* valTI = resolveTypeInfo(typeParams[1]);
            auto fullName = baseName + "<" + keyTI->name + ", " + valTI->name + ">";
            if (auto* existing = typeRegistry_.lookup(fullName))
                return existing;

            TypeInfo ti;
            ti.name = fullName;
            ti.kind = TypeKind::Extended;
            ti.bitWidth = 0;
            ti.isSigned = false;
            ti.builtinSuffix = keyTI->builtinSuffix + "_" + valTI->builtinSuffix;
            ti.keyType = keyTI;
            ti.valueType = valTI;
            ti.extendedKind = baseName;
            typeRegistry_.registerType(std::move(ti));
            return typeRegistry_.lookup(fullName);
        }
        return nullptr;
    }

    // Unwrap array dimensions [][]...T to reach the base primitive type
    auto* inner = ctx;
    while (!inner->typeSpec().empty())
        inner = inner->typeSpec(0);
    auto name = inner->getText();

    // cstring is a built-in alias for *char
    if (name == "cstring") {
        auto ptrName = "*char";
        if (auto* existing = typeRegistry_.lookup(ptrName))
            return existing;
        auto* charTI = typeRegistry_.lookup("char");
        TypeInfo ti;
        ti.name = ptrName;
        ti.kind = TypeKind::Pointer;
        ti.bitWidth = 0;
        ti.isSigned = false;
        ti.builtinSuffix = "ptr";
        ti.pointeeType = charTI;
        typeRegistry_.registerType(std::move(ti));
        return typeRegistry_.lookup(ptrName);
    }

    auto* ti  = typeRegistry_.lookup(name);
    if (!ti) {
        std::cerr << "lux: unknown type '" << name << "'\n";
        return typeRegistry_.lookup("int32");
    }
    return ti;
}

bool IRGen::isPointerType(LuxParser::TypeSpecContext* ctx) {
    return ctx->STAR() != nullptr;
}

const TypeInfo* IRGen::resolveExprTypeInfo(LuxParser::ExpressionContext* ctx) {
    // Identifier: look up in locals, then C globals
    if (auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(ctx)) {
        auto name = ident->IDENTIFIER()->getText();
        auto it = locals_.find(name);
        if (it != locals_.end()) return it->second.typeInfo;
        auto git = cGlobals_.find(name);
        if (git != cGlobals_.end()) return git->second.type;
        return nullptr;
    }

    // Dot access: resolve base struct/union, find field type
    if (auto* dot = dynamic_cast<LuxParser::FieldAccessExprContext*>(ctx)) {
        auto* baseTI = resolveExprTypeInfo(dot->expression());
        if (!baseTI || (baseTI->kind != TypeKind::Struct && baseTI->kind != TypeKind::Union)) return nullptr;
        auto fname = dot->IDENTIFIER()->getText();
        for (auto& f : baseTI->fields) {
            if (f.name == fname) return f.typeInfo;
        }
        return nullptr;
    }

    // Arrow access: resolve base pointer, dereference, find field type
    if (auto* arrow = dynamic_cast<LuxParser::ArrowAccessExprContext*>(ctx)) {
        auto* baseTI = resolveExprTypeInfo(arrow->expression());
        if (!baseTI || baseTI->kind != TypeKind::Pointer) return nullptr;
        auto* structTI = baseTI->pointeeType;
        if (!structTI || (structTI->kind != TypeKind::Struct && structTI->kind != TypeKind::Union)) return nullptr;
        auto fname = arrow->IDENTIFIER()->getText();
        for (auto& f : structTI->fields) {
            if (f.name == fname) return f.typeInfo;
        }
        return nullptr;
    }

    // Additive: ptr + int → pointer type preserved
    if (auto* add = dynamic_cast<LuxParser::AddSubExprContext*>(ctx)) {
        auto* lhsTI = resolveExprTypeInfo(add->expression(0));
        if (lhsTI && lhsTI->kind == TypeKind::Pointer) return lhsTI;
        auto* rhsTI = resolveExprTypeInfo(add->expression(1));
        if (rhsTI && rhsTI->kind == TypeKind::Pointer) return rhsTI;
        return lhsTI;
    }

    // Cast: resolve target type
    if (auto* cast = dynamic_cast<LuxParser::CastExprContext*>(ctx)) {
        return resolveTypeInfo(cast->typeSpec());
    }

    // Paren: unwrap
    if (auto* paren = dynamic_cast<LuxParser::ParenExprContext*>(ctx)) {
        return resolveExprTypeInfo(paren->expression());
    }

    // Index: array[i] → element type (which is the variable's typeInfo)
    if (auto* idx = dynamic_cast<LuxParser::IndexExprContext*>(ctx)) {
        // Walk down to the root variable
        auto* current = static_cast<LuxParser::ExpressionContext*>(idx);
        while (auto* idxCtx = dynamic_cast<LuxParser::IndexExprContext*>(current))
            current = idxCtx->expression(0);
        auto* ident = dynamic_cast<LuxParser::IdentExprContext*>(current);
        if (ident) {
            auto it = locals_.find(ident->IDENTIFIER()->getText());
            if (it != locals_.end())
                return it->second.typeInfo;  // element type of the array
        }
        return nullptr;
    }

    return nullptr;
}

unsigned IRGen::countArrayDims(LuxParser::TypeSpecContext* ctx) {
    unsigned dims = 0;
    while (!ctx->typeSpec().empty()) {
        // Only count [] array dimensions, not * pointer prefixes
        if (ctx->STAR())
            break;
        dims++;
        ctx = ctx->typeSpec(0);
    }
    return dims;
}

llvm::ArrayType* IRGen::buildTargetArrayType(llvm::Type* srcTy, llvm::Type* elemTy) {
    auto* srcArr  = llvm::cast<llvm::ArrayType>(srcTy);
    auto  count   = srcArr->getNumElements();
    auto* srcElem = srcArr->getElementType();
    if (srcElem->isArrayTy())
        return llvm::ArrayType::get(buildTargetArrayType(srcElem, elemTy), count);
    return llvm::ArrayType::get(elemTy, count);
}

void IRGen::storeArrayElements(llvm::Value* src, llvm::Value* destPtr,
                                llvm::Type* destArrTy, const TypeInfo* elemTI,
                                unsigned dims) {
    auto* arrTy = llvm::cast<llvm::ArrayType>(destArrTy);
    unsigned count = arrTy->getNumElements();
    auto* i64Ty = llvm::Type::getInt64Ty(*context_);
    auto* zero  = llvm::ConstantInt::get(i64Ty, 0);

    for (unsigned i = 0; i < count; i++) {
        auto* elem = builder_->CreateExtractValue(src, {i});
        auto* idx  = llvm::ConstantInt::get(i64Ty, i);
        auto* gep  = builder_->CreateGEP(destArrTy, destPtr, { zero, idx });

        if (dims > 1) {
            storeArrayElements(elem, gep, arrTy->getElementType(), elemTI, dims - 1);
        } else {
            auto* targetElemTy = arrTy->getElementType();
            if (elem->getType() != targetElemTy) {
                if (elem->getType()->isIntegerTy() && targetElemTy->isIntegerTy())
                    elem = builder_->CreateIntCast(elem, targetElemTy, elemTI->isSigned);
                else if (elem->getType()->isFloatingPointTy() && targetElemTy->isFloatingPointTy()) {
                    if (elem->getType()->getPrimitiveSizeInBits() > targetElemTy->getPrimitiveSizeInBits())
                        elem = builder_->CreateFPTrunc(elem, targetElemTy);
                    else
                        elem = builder_->CreateFPExt(elem, targetElemTy);
                }
            }
            builder_->CreateStore(elem, gep);
        }
    }
}

// Declare (or retrieve an already-declared) external C function in the module.
llvm::FunctionCallee IRGen::declareBuiltin(const std::string&           name,
                                           llvm::Type*                  retType,
                                           llvm::ArrayRef<llvm::Type*>  argTypes) {
    if (auto* existing = module_->getFunction(name)) {
        return { existing->getFunctionType(), existing };
    }
    auto* fnType = llvm::FunctionType::get(retType, argTypes, false);
    return module_->getOrInsertFunction(name, fnType);
}

llvm::StructType* IRGen::getOrCreateVecStructType() {
    auto* existing = llvm::StructType::getTypeByName(*context_, "lux_vec_header");
    if (existing) return existing;

    auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
    auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
    return llvm::StructType::create(*context_,
        { ptrTy, usizeTy, usizeTy }, "lux_vec_header");
}

llvm::StructType* IRGen::getOrCreateMapStructType() {
    auto* existing = llvm::StructType::getTypeByName(*context_, "lux_map_header");
    if (existing) return existing;

    auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
    auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
    return llvm::StructType::create(*context_,
        { ptrTy, ptrTy, ptrTy, ptrTy,
          usizeTy, usizeTy, usizeTy, usizeTy }, "lux_map_header");
}

llvm::StructType* IRGen::getOrCreateSetStructType() {
    auto* existing = llvm::StructType::getTypeByName(*context_, "lux_set_header");
    if (existing) return existing;

    auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
    auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
    return llvm::StructType::create(*context_,
        { ptrTy, ptrTy, ptrTy,
          usizeTy, usizeTy, usizeTy }, "lux_set_header");
}

std::string IRGen::getVecSuffix(const TypeInfo* elemTI) {
    return elemTI->builtinSuffix;
}

// ── Defer infrastructure ────────────────────────────────────────────────────

std::any IRGen::visitDeferStmt(LuxParser::DeferStmtContext* ctx) {
    DeferredStmt ds;
    if (ctx->callStmt())
        ds.callCtx = ctx->callStmt();
    else if (ctx->exprStmt())
        ds.exprCtx = ctx->exprStmt();
    deferStack_.push_back(ds);
    return {};
}

void IRGen::emitDeferredCleanups() {
    for (auto it = deferStack_.rbegin(); it != deferStack_.rend(); ++it) {
        if (it->callCtx)
            visit(it->callCtx);
        else if (it->exprCtx)
            visit(it->exprCtx);
    }
}

void IRGen::emitAutoCleanups(const std::string& skipVar) {
    auto* voidTy = llvm::Type::getVoidTy(*context_);
    auto* ptrTy  = llvm::PointerType::getUnqual(*context_);

    for (auto& [name, info] : locals_) {
        if (name == skipVar) continue;
        if (info.isParam) continue;  // borrowed from caller, don't free
        if (!info.typeInfo || info.typeInfo->kind != TypeKind::Extended) continue;

        std::string freeFuncName;
        if (info.typeInfo->extendedKind == "Vec") {
            auto suffix = info.typeInfo->elementType
                              ? info.typeInfo->elementType->builtinSuffix
                              : info.typeInfo->builtinSuffix;
            freeFuncName = "lux_vec_free_" + suffix;
        } else if (info.typeInfo->extendedKind == "Map") {
            freeFuncName = "lux_map_free_" + info.typeInfo->builtinSuffix;
        } else if (info.typeInfo->extendedKind == "Set") {
            auto suffix = info.typeInfo->elementType
                              ? info.typeInfo->elementType->builtinSuffix
                              : info.typeInfo->builtinSuffix;
            freeFuncName = "lux_set_free_" + suffix;
        } else {
            continue;
        }

        auto callee = declareBuiltin(freeFuncName, voidTy, {ptrTy});
        builder_->CreateCall(callee, {info.alloca});
    }
}

void IRGen::emitAllCleanups(const std::string& skipVar) {
    emitDeferredCleanups();
    emitAutoCleanups(skipVar);
}

std::any
IRGen::visitMethodCallExpr(LuxParser::MethodCallExprContext* ctx) {
    auto methodName = ctx->IDENTIFIER()->getText();

    // Resolve receiver type info BEFORE evaluating receiver
    const TypeInfo* receiverTI = nullptr;
    unsigned recvArrayDims = 0;
    auto* baseExpr = ctx->expression();
    std::string receiverVarName;
    if (auto* identBase = dynamic_cast<LuxParser::IdentExprContext*>(baseExpr)) {
        receiverVarName = identBase->IDENTIFIER()->getText();
        auto it = locals_.find(receiverVarName);
        if (it != locals_.end()) {
            receiverTI = it->second.typeInfo;
            recvArrayDims = it->second.arrayDims;
        }
    }

    // ── Variadic parameter method dispatch ───────────────────────────
    if (!receiverVarName.empty()) {
        auto vit = variadicParams_.find(receiverVarName);
        if (vit != variadicParams_.end()) {
            if (methodName == "len") {
                auto* i64Ty = llvm::Type::getInt64Ty(*context_);
                return static_cast<llvm::Value*>(
                    builder_->CreateLoad(i64Ty, vit->second.lenAlloca, "var_len"));
            }
            std::cerr << "lux: variadic parameter '" << receiverVarName
                      << "' has no method '" << methodName << "'\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }
    }

    // ── Extended type method dispatch ────────────────────────────────
    if (receiverTI && receiverTI->kind == TypeKind::Extended) {
        auto* extDesc = extTypeRegistry_.lookup(receiverTI->extendedKind);
        if (!extDesc) {
            std::cerr << "lux: unknown extended type '" << receiverTI->extendedKind << "'\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        // Find method descriptor
        const MethodDescriptor* desc = nullptr;
        for (auto& md : extDesc->methods) {
            if (md.name == methodName) { desc = &md; break; }
        }
        if (!desc) {
            std::cerr << "lux: unknown method '" << methodName
                      << "' on type '" << receiverTI->name << "'\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        // Collect argument values
        std::vector<llvm::Value*> args;
        if (auto* argList = ctx->argList()) {
            for (auto* argExpr : argList->expression()) {
                args.push_back(std::any_cast<llvm::Value*>(visit(argExpr)));
            }
        }

        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);

        // Determine struct type and suffix based on extended kind
        llvm::StructType* extStructTy;
        std::string suffix;
        if (receiverTI->extendedKind == "Map") {
            extStructTy = getOrCreateMapStructType();
            suffix = receiverTI->builtinSuffix; // "keySuffix_valSuffix"
        } else if (receiverTI->extendedKind == "Set") {
            extStructTy = getOrCreateSetStructType();
            suffix = receiverTI->elementType
                         ? receiverTI->elementType->builtinSuffix
                         : receiverTI->builtinSuffix;
        } else {
            extStructTy = getOrCreateVecStructType();
            suffix = receiverTI->elementType
                         ? receiverTI->elementType->builtinSuffix
                         : receiverTI->builtinSuffix;
        }
        auto cFuncName = extDesc->cPrefix + "_" + methodName + "_" + suffix;

        // Get the alloca pointer for the receiver variable
        llvm::Value* recvPtr = nullptr;
        if (!receiverVarName.empty()) {
            auto it = locals_.find(receiverVarName);
            if (it != locals_.end())
                recvPtr = it->second.alloca;
        }
        if (!recvPtr) {
            std::cerr << "lux: cannot get address of extended type for method call\n";
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        // ── Map.keys() / Map.values() / Set.values() — out-param returning Vec
        if ((receiverTI->extendedKind == "Map" &&
             (methodName == "keys" || methodName == "values")) ||
            (receiverTI->extendedKind == "Set" && methodName == "values")) {
            auto* vecTy     = getOrCreateVecStructType();
            auto* voidTy    = llvm::Type::getVoidTy(*context_);
            auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr,
                                                     methodName + "_out");
            auto callee = declareBuiltin(cFuncName, voidTy, {ptrTy, ptrTy});
            builder_->CreateCall(callee, {recvPtr, outAlloca});
            auto* result = builder_->CreateLoad(vecTy, outAlloca,
                                                methodName + "_val");
            return static_cast<llvm::Value*>(result);
        }

        // ── Vec.clone() — out-param returning a new Vec of the same type
        if (receiverTI->extendedKind == "Vec" && methodName == "clone") {
            auto* vecTy     = getOrCreateVecStructType();
            auto* voidTy    = llvm::Type::getVoidTy(*context_);
            auto* outAlloca = builder_->CreateAlloca(vecTy, nullptr, "clone_out");
            auto callee = declareBuiltin(cFuncName, voidTy, {ptrTy, ptrTy});
            builder_->CreateCall(callee, {recvPtr, outAlloca});
            auto* result = builder_->CreateLoad(vecTy, outAlloca, "clone_val");
            return static_cast<llvm::Value*>(result);
        }

        // Resolve placeholder LLVM types
        llvm::Type* elemLLTy = nullptr;
        llvm::Type* keyLLTy  = nullptr;
        llvm::Type* valLLTy  = nullptr;

        if (receiverTI->elementType)
            elemLLTy = receiverTI->elementType->toLLVMType(*context_, module_->getDataLayout());
        if (receiverTI->keyType)
            keyLLTy = receiverTI->keyType->toLLVMType(*context_, module_->getDataLayout());
        if (receiverTI->valueType)
            valLLTy = receiverTI->valueType->toLLVMType(*context_, module_->getDataLayout());

        // Resolve return type
        llvm::Type* retTy;
        if (desc->returnType == "void")
            retTy = llvm::Type::getVoidTy(*context_);
        else if (desc->returnType == "_self")
            retTy = extStructTy;
        else if (desc->returnType == "_elem")
            retTy = elemLLTy;
        else if (desc->returnType == "_key")
            retTy = keyLLTy;
        else if (desc->returnType == "_val")
            retTy = valLLTy;
        else if (desc->returnType == "bool")
            retTy = llvm::Type::getInt1Ty(*context_);
        else if (desc->returnType == "usize")
            retTy = usizeTy;
        else if (desc->returnType == "int64")
            retTy = llvm::Type::getInt64Ty(*context_);
        else if (desc->returnType == "float64")
            retTy = llvm::Type::getDoubleTy(*context_);
        else if (desc->returnType == "string") {
            auto* sPtrTy = llvm::PointerType::getUnqual(*context_);
            retTy = llvm::StructType::get(*context_, { sPtrTy, usizeTy });
        } else {
            auto* rTI = typeRegistry_.lookup(desc->returnType);
            retTy = rTI ? rTI->toLLVMType(*context_, module_->getDataLayout())
                        : llvm::Type::getInt32Ty(*context_);
        }

        // For bool-returning methods in C (return int), use i32 for ABI
        bool boolReturn = (desc->returnType == "bool");
        llvm::Type* cRetTy = boolReturn
            ? llvm::Type::getInt32Ty(*context_) : retTy;

        // Build C function argument list: first arg is always receiver ptr
        std::vector<llvm::Value*> callArgs = { recvPtr };
        std::vector<llvm::Type*> paramLLTypes = { ptrTy };

        for (size_t i = 0; i < desc->paramTypes.size(); i++) {
            auto& pt = desc->paramTypes[i];
            llvm::Type* paramTy;
            if (pt == "_elem")       paramTy = elemLLTy;
            else if (pt == "_key")   paramTy = keyLLTy;
            else if (pt == "_val")   paramTy = valLLTy;
            else if (pt == "_self")  paramTy = ptrTy;
            else if (pt == "usize")  paramTy = usizeTy;
            else if (pt == "int32")  paramTy = llvm::Type::getInt32Ty(*context_);
            else if (pt == "string") {
                auto* sPtrTy = llvm::PointerType::getUnqual(*context_);
                paramTy = llvm::StructType::get(*context_, { sPtrTy, usizeTy });
            } else {
                auto* pTI = typeRegistry_.lookup(pt);
                paramTy = pTI ? pTI->toLLVMType(*context_, module_->getDataLayout())
                              : llvm::Type::getInt32Ty(*context_);
            }
            paramLLTypes.push_back(paramTy);

            // Cast argument if needed
            llvm::Value* argVal = args[i];
            if (pt == "_self") {
                // _self params expect a pointer to the struct, not the struct
                // by value. Try to get the alloca from the argument expression.
                if (auto* argList = ctx->argList()) {
                    auto* argExpr = argList->expression(i);
                    if (auto* identArg = dynamic_cast<LuxParser::IdentExprContext*>(argExpr)) {
                        auto ait = locals_.find(identArg->IDENTIFIER()->getText());
                        if (ait != locals_.end())
                            argVal = ait->second.alloca;
                    }
                }
            } else if (argVal->getType() != paramTy) {
                if (argVal->getType()->isIntegerTy() && paramTy->isIntegerTy())
                    argVal = builder_->CreateIntCast(argVal, paramTy, false);
            }
            callArgs.push_back(argVal);
        }

        auto callee = declareBuiltin(cFuncName, cRetTy, paramLLTypes);

        if (retTy->isVoidTy()) {
            builder_->CreateCall(callee, callArgs);
            return static_cast<llvm::Value*>(
                llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
        }

        auto* result = builder_->CreateCall(callee, callArgs, "ext_method");

        // Convert C int → i1 for bool returns
        if (boolReturn)
            return static_cast<llvm::Value*>(
                builder_->CreateICmpNE(result,
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0)));

        return static_cast<llvm::Value*>(result);
    }

    // ── Non-extended method call (original path) ─────────────────────

    // ── Struct method via `extend` block ────────────────────────────
    if (receiverTI && receiverTI->kind == TypeKind::Struct) {
        auto smIt = structMethods_.find(receiverTI->name);
        if (smIt != structMethods_.end()) {
            auto mIt = smIt->second.find(methodName);
            if (mIt != smIt->second.end()) {
                auto* fn = mIt->second;

                // Collect argument values
                std::vector<llvm::Value*> callArgs;

                // First arg: pointer to the struct instance (&self)
                auto it = locals_.find(receiverVarName);
                if (it != locals_.end()) {
                    callArgs.push_back(it->second.alloca);
                } else {
                    // Fallback: evaluate and take address
                    auto* receiverVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));
                    callArgs.push_back(receiverVal);
                }

                // Remaining args
                if (auto* argList = ctx->argList()) {
                    auto* fnType = fn->getFunctionType();
                    size_t paramIdx = 1; // skip &self
                    for (auto* argExpr : argList->expression()) {
                        auto* argVal = std::any_cast<llvm::Value*>(visit(argExpr));
                        if (paramIdx < fnType->getNumParams()) {
                            auto* paramTy = fnType->getParamType(paramIdx);
                            if (argVal->getType() != paramTy) {
                                if (argVal->getType()->isIntegerTy() && paramTy->isIntegerTy())
                                    argVal = builder_->CreateIntCast(argVal, paramTy, true);
                                else if (argVal->getType()->isFloatingPointTy() && paramTy->isFloatingPointTy())
                                    argVal = builder_->CreateFPCast(argVal, paramTy);
                            }
                        }
                        callArgs.push_back(argVal);
                        paramIdx++;
                    }
                }

                auto* retTy = fn->getReturnType();
                if (retTy->isVoidTy()) {
                    builder_->CreateCall(fn, callArgs);
                    return static_cast<llvm::Value*>(
                        llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
                }
                return static_cast<llvm::Value*>(
                    builder_->CreateCall(fn, callArgs, "method_result"));
            }
        }
    }

    // ── String method dispatch ───────────────────────────────────────
    if (receiverTI && receiverTI->kind == TypeKind::String && !receiverVarName.empty()) {
        auto* ptrTy   = llvm::PointerType::getUnqual(*context_);
        auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
        auto* i1Ty    = llvm::Type::getInt1Ty(*context_);
        auto* i8Ty    = llvm::Type::getInt8Ty(*context_);
        auto* i32Ty   = llvm::Type::getInt32Ty(*context_);
        auto* i64Ty   = llvm::Type::getInt64Ty(*context_);
        auto* strTy   = llvm::StructType::get(*context_, {ptrTy, usizeTy});

        // Load receiver string struct from its alloca
        auto recvIt = locals_.find(receiverVarName);
        if (recvIt != locals_.end()) {
            auto* strVal = builder_->CreateLoad(strTy, recvIt->second.alloca,
                                                receiverVarName + "_strval");
            auto* strPtr = builder_->CreateExtractValue(strVal, 0, "str_ptr");
            auto* strLen = builder_->CreateExtractValue(strVal, 1, "str_len");

            // Collect extra arguments
            std::vector<llvm::Value*> mArgs;
            if (auto* argList = ctx->argList())
                for (auto* e : argList->expression())
                    mArgs.push_back(std::any_cast<llvm::Value*>(visit(e)));

            // Helper: rebuild string struct from C return value { ptr, len }
            auto buildStr = [&](llvm::Value* ret) -> llvm::Value* {
                auto* rPtr = builder_->CreateExtractValue(ret, 0, "sret_ptr");
                auto* rLen = builder_->CreateExtractValue(ret, 1, "sret_len");
                llvm::Value* s = llvm::UndefValue::get(strTy);
                s = builder_->CreateInsertValue(s, rPtr, 0);
                s = builder_->CreateInsertValue(s, rLen, 1);
                return s;
            };

            // Helper: expand string fat-pointer arg into (ptr, len)
            auto extractStr = [&](llvm::Value* sv, std::vector<llvm::Value*>& out) {
                out.push_back(builder_->CreateExtractValue(sv, 0, "arg_ptr"));
                out.push_back(builder_->CreateExtractValue(sv, 1, "arg_len"));
            };

            if (methodName == "len") {
                return static_cast<llvm::Value*>(strLen);
            }
            if (methodName == "isEmpty") {
                auto* zero = llvm::ConstantInt::get(usizeTy, 0);
                return static_cast<llvm::Value*>(
                    builder_->CreateICmpEQ(strLen, zero, "str_empty"));
            }
            if (methodName == "toString") {
                return static_cast<llvm::Value*>(strVal);
            }
            if (methodName == "front") {
                auto* elem = builder_->CreateGEP(i8Ty, strPtr,
                    llvm::ConstantInt::get(usizeTy, 0), "front_ptr");
                return static_cast<llvm::Value*>(builder_->CreateLoad(i8Ty, elem, "front"));
            }
            if (methodName == "back") {
                auto* idx = builder_->CreateSub(strLen,
                    llvm::ConstantInt::get(usizeTy, 1), "back_idx");
                auto* elem = builder_->CreateGEP(i8Ty, strPtr, idx, "back_ptr");
                return static_cast<llvm::Value*>(builder_->CreateLoad(i8Ty, elem, "back"));
            }
            if (methodName == "at") {
                auto* idx = mArgs[0];
                if (idx->getType() != usizeTy)
                    idx = builder_->CreateIntCast(idx, usizeTy, false);
                auto* elem = builder_->CreateGEP(i8Ty, strPtr, idx, "at_ptr");
                return static_cast<llvm::Value*>(builder_->CreateLoad(i8Ty, elem, "at"));
            }
            // (receiver, string) → bool
            if (methodName == "contains" || methodName == "startsWith" ||
                methodName == "endsWith") {
                static const std::unordered_map<std::string, std::string> boolFns = {
                    {"contains",   "lux_contains"},
                    {"startsWith", "lux_startsWith"},
                    {"endsWith",   "lux_endsWith"},
                };
                std::vector<llvm::Value*> callArgs = {strPtr, strLen};
                extractStr(mArgs[0], callArgs);
                auto callee = declareBuiltin(boolFns.at(methodName), i32Ty,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                auto* r = builder_->CreateCall(callee, callArgs, methodName);
                return static_cast<llvm::Value*>(
                    builder_->CreateICmpNE(r, llvm::ConstantInt::get(i32Ty, 0)));
            }
            // (receiver, string) → int64
            if (methodName == "indexOf" || methodName == "lastIndexOf") {
                static const std::unordered_map<std::string, std::string> idxFns = {
                    {"indexOf",     "lux_indexOf"},
                    {"lastIndexOf", "lux_lastIndexOf"},
                };
                std::vector<llvm::Value*> callArgs = {strPtr, strLen};
                extractStr(mArgs[0], callArgs);
                auto callee = declareBuiltin(idxFns.at(methodName), i64Ty,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                return static_cast<llvm::Value*>(
                    builder_->CreateCall(callee, callArgs, methodName));
            }
            if (methodName == "count") {
                std::vector<llvm::Value*> callArgs = {strPtr, strLen};
                extractStr(mArgs[0], callArgs);
                auto callee = declareBuiltin("lux_count", usizeTy,
                    {ptrTy, usizeTy, ptrTy, usizeTy});
                return static_cast<llvm::Value*>(
                    builder_->CreateCall(callee, callArgs, "count"));
            }
            // No-arg string → string transformations
            if (methodName == "toUpper"  || methodName == "toLower"  ||
                methodName == "trim"     || methodName == "trimLeft" ||
                methodName == "trimRight"|| methodName == "reverse") {
                static const std::unordered_map<std::string, std::string> xformFns = {
                    {"toUpper",   "lux_toUpper"},
                    {"toLower",   "lux_toLower"},
                    {"trim",      "lux_trim"},
                    {"trimLeft",  "lux_trimLeft"},
                    {"trimRight", "lux_trimRight"},
                    {"reverse",   "lux_reverse"},
                };
                auto callee = declareBuiltin(xformFns.at(methodName), strTy, {ptrTy, usizeTy});
                auto* r = builder_->CreateCall(callee, {strPtr, strLen}, methodName);
                return static_cast<llvm::Value*>(buildStr(r));
            }
            if (methodName == "substring") {
                auto* start  = mArgs[0];
                auto* length = mArgs[1];
                if (start->getType()  != usizeTy) start  = builder_->CreateIntCast(start,  usizeTy, false);
                if (length->getType() != usizeTy) length = builder_->CreateIntCast(length, usizeTy, false);
                auto callee = declareBuiltin("lux_substring", strTy,
                    {ptrTy, usizeTy, usizeTy, usizeTy});
                auto* r = builder_->CreateCall(callee, {strPtr, strLen, start, length}, "substring");
                return static_cast<llvm::Value*>(buildStr(r));
            }
            if (methodName == "slice") {
                auto* start = mArgs[0];
                auto* end   = mArgs[1];
                if (start->getType() != i64Ty) start = builder_->CreateIntCast(start, i64Ty, true);
                if (end->getType()   != i64Ty) end   = builder_->CreateIntCast(end,   i64Ty, true);
                auto callee = declareBuiltin("lux_slice", strTy, {ptrTy, usizeTy, i64Ty, i64Ty});
                auto* r = builder_->CreateCall(callee, {strPtr, strLen, start, end}, "slice");
                return static_cast<llvm::Value*>(buildStr(r));
            }
            if (methodName == "repeat") {
                auto* n = mArgs[0];
                if (n->getType() != usizeTy) n = builder_->CreateIntCast(n, usizeTy, false);
                auto callee = declareBuiltin("lux_repeat", strTy, {ptrTy, usizeTy, usizeTy});
                auto* r = builder_->CreateCall(callee, {strPtr, strLen, n}, "repeat");
                return static_cast<llvm::Value*>(buildStr(r));
            }
            if (methodName == "padLeft" || methodName == "padRight") {
                std::string fname = (methodName == "padLeft") ? "lux_padLeft" : "lux_padRight";
                auto* width = mArgs[0];
                auto* fill  = mArgs[1];
                if (width->getType() != usizeTy) width = builder_->CreateIntCast(width, usizeTy, false);
                auto callee = declareBuiltin(fname, strTy, {ptrTy, usizeTy, usizeTy, i8Ty});
                auto* r = builder_->CreateCall(callee, {strPtr, strLen, width, fill}, methodName);
                return static_cast<llvm::Value*>(buildStr(r));
            }
            if (methodName == "replace" || methodName == "replaceFirst") {
                std::string fname = (methodName == "replace") ? "lux_replace" : "lux_replaceFirst";
                std::vector<llvm::Value*> callArgs = {strPtr, strLen};
                extractStr(mArgs[0], callArgs);  // old
                extractStr(mArgs[1], callArgs);  // rep
                auto callee = declareBuiltin(fname, strTy,
                    {ptrTy, usizeTy, ptrTy, usizeTy, ptrTy, usizeTy});
                auto* r = builder_->CreateCall(callee, callArgs, methodName);
                return static_cast<llvm::Value*>(buildStr(r));
            }
        }
    }

    // Evaluate receiver
    auto* receiverVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    // Collect arguments
    std::vector<llvm::Value*> args;
    if (auto* argList = ctx->argList()) {
        for (auto* argExpr : argList->expression()) {
            args.push_back(std::any_cast<llvm::Value*>(visit(argExpr)));
        }
    }

    // Look up the method descriptor
    const MethodDescriptor* desc = nullptr;
    if (recvArrayDims > 0)
        desc = methodRegistry_.lookupArrayMethod(methodName);
    else if (receiverTI)
        desc = methodRegistry_.lookup(receiverTI->kind, methodName);

    if (!desc) {
        std::cerr << "lux: unknown method '" << methodName << "'\n";
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
    }

    // ── Array method codegen ─────────────────────────────────────────
    if (recvArrayDims > 0 && !receiverVarName.empty()) {
        auto it = locals_.find(receiverVarName);
        if (it != locals_.end()) {
            auto* arrAlloca = it->second.alloca;
            auto* allocaTy  = arrAlloca->getAllocatedType();
            auto* arrTy     = llvm::dyn_cast<llvm::ArrayType>(allocaTy);
            if (!arrTy) {
                std::cerr << "lux: expected array type for '" << receiverVarName << "'\n";
                return static_cast<llvm::Value*>(
                    llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
            }
            uint64_t arrLen = arrTy->getNumElements();
            auto* elemTy    = arrTy->getElementType();
            auto* i64Ty     = llvm::Type::getInt64Ty(*context_);
            auto* usizeTy   = module_->getDataLayout().getIntPtrType(*context_);

            // Helper: cast a value to elemTy if types differ
            auto castToElem = [&](llvm::Value* v) -> llvm::Value* {
                if (v->getType() == elemTy) return v;
                if (elemTy->isIntegerTy() && v->getType()->isIntegerTy())
                    return builder_->CreateIntCast(v, elemTy, true);
                if (elemTy->isFloatingPointTy() && v->getType()->isFloatingPointTy())
                    return builder_->CreateFPCast(v, elemTy);
                if (elemTy->isFloatingPointTy() && v->getType()->isIntegerTy())
                    return builder_->CreateSIToFP(v, elemTy);
                if (elemTy->isIntegerTy() && v->getType()->isFloatingPointTy())
                    return builder_->CreateFPToSI(v, elemTy);
                return v;
            };

            if (methodName == "len") {
                return static_cast<llvm::Value*>(
                    llvm::ConstantInt::get(usizeTy, arrLen));
            }
            if (methodName == "isEmpty") {
                return static_cast<llvm::Value*>(
                    llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), arrLen == 0));
            }
            if (methodName == "first") {
                auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                auto* gep  = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, zero}, "first_ptr");
                return static_cast<llvm::Value*>(builder_->CreateLoad(elemTy, gep, "first"));
            }
            if (methodName == "last") {
                auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                auto* last = llvm::ConstantInt::get(i64Ty, arrLen - 1);
                auto* gep  = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, last}, "last_ptr");
                return static_cast<llvm::Value*>(builder_->CreateLoad(elemTy, gep, "last"));
            }
            if (methodName == "at") {
                if (args.empty()) {
                    std::cerr << "lux: at() requires an index argument\n";
                    return static_cast<llvm::Value*>(llvm::UndefValue::get(elemTy));
                }
                auto* idx  = args[0];
                if (idx->getType() != i64Ty)
                    idx = builder_->CreateIntCast(idx, i64Ty, true);
                auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                auto* gep  = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, idx}, "at_ptr");
                return static_cast<llvm::Value*>(builder_->CreateLoad(elemTy, gep, "at"));
            }
            if (methodName == "contains") {
                if (args.empty()) {
                    std::cerr << "lux: contains() requires a value argument\n";
                    return static_cast<llvm::Value*>(
                        llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 0));
                }
                auto* needle = castToElem(args[0]);
                auto* boolTy = llvm::Type::getInt1Ty(*context_);
                auto* resultAlloca = builder_->CreateAlloca(boolTy, nullptr, "contains_res");
                builder_->CreateStore(llvm::ConstantInt::get(boolTy, 0), resultAlloca);

                auto* condBB   = llvm::BasicBlock::Create(*context_, "contains.cond", currentFunction_);
                auto* bodyBB   = llvm::BasicBlock::Create(*context_, "contains.body", currentFunction_);
                auto* foundBB  = llvm::BasicBlock::Create(*context_, "contains.found", currentFunction_);
                auto* nextBB   = llvm::BasicBlock::Create(*context_, "contains.next", currentFunction_);
                auto* endBB    = llvm::BasicBlock::Create(*context_, "contains.end", currentFunction_);

                auto* idxAlloca = builder_->CreateAlloca(i64Ty, nullptr, "contains_idx");
                builder_->CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);
                builder_->CreateBr(condBB);

                builder_->SetInsertPoint(condBB);
                auto* idx  = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
                auto* cond = builder_->CreateICmpULT(idx, llvm::ConstantInt::get(i64Ty, arrLen), "cmp");
                builder_->CreateCondBr(cond, bodyBB, endBB);

                builder_->SetInsertPoint(bodyBB);
                auto* curIdx = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
                auto* zero   = llvm::ConstantInt::get(i64Ty, 0);
                auto* gep    = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, curIdx}, "elem_ptr");
                auto* elem   = builder_->CreateLoad(elemTy, gep, "elem");
                llvm::Value* eq;
                if (elemTy->isIntegerTy())
                    eq = builder_->CreateICmpEQ(elem, needle, "eq");
                else if (elemTy->isFloatingPointTy())
                    eq = builder_->CreateFCmpOEQ(elem, needle, "eq");
                else
                    eq = builder_->CreateICmpEQ(elem, needle, "eq");
                builder_->CreateCondBr(eq, foundBB, nextBB);

                builder_->SetInsertPoint(foundBB);
                builder_->CreateStore(llvm::ConstantInt::get(boolTy, 1), resultAlloca);
                builder_->CreateBr(endBB);

                builder_->SetInsertPoint(nextBB);
                auto* nextIdx = builder_->CreateAdd(
                    builder_->CreateLoad(i64Ty, idxAlloca, "idx"),
                    llvm::ConstantInt::get(i64Ty, 1), "next_idx");
                builder_->CreateStore(nextIdx, idxAlloca);
                builder_->CreateBr(condBB);

                builder_->SetInsertPoint(endBB);
                return static_cast<llvm::Value*>(
                    builder_->CreateLoad(boolTy, resultAlloca, "contains"));
            }
            if (methodName == "indexOf" || methodName == "lastIndexOf") {
                if (args.empty()) {
                    std::cerr << "lux: " << methodName << "() requires a value argument\n";
                    return static_cast<llvm::Value*>(
                        llvm::ConstantInt::get(i64Ty, -1));
                }
                auto* needle = castToElem(args[0]);
                auto* resultAlloca = builder_->CreateAlloca(i64Ty, nullptr, "indexOf_res");
                builder_->CreateStore(llvm::ConstantInt::get(i64Ty, -1ULL), resultAlloca);

                if (methodName == "indexOf") {
                    // Forward scan: 0..len, break on first match
                    auto* condBB = llvm::BasicBlock::Create(*context_, "indexOf.cond", currentFunction_);
                    auto* bodyBB = llvm::BasicBlock::Create(*context_, "indexOf.body", currentFunction_);
                    auto* foundBB = llvm::BasicBlock::Create(*context_, "indexOf.found", currentFunction_);
                    auto* nextBB = llvm::BasicBlock::Create(*context_, "indexOf.next", currentFunction_);
                    auto* endBB  = llvm::BasicBlock::Create(*context_, "indexOf.end", currentFunction_);

                    auto* idxAlloca = builder_->CreateAlloca(i64Ty, nullptr, "idx");
                    builder_->CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);
                    builder_->CreateBr(condBB);

                    builder_->SetInsertPoint(condBB);
                    auto* idx = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
                    auto* cmp = builder_->CreateICmpULT(idx, llvm::ConstantInt::get(i64Ty, arrLen), "cmp");
                    builder_->CreateCondBr(cmp, bodyBB, endBB);

                    builder_->SetInsertPoint(bodyBB);
                    auto* curIdx = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
                    auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                    auto* gep = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, curIdx}, "elem_ptr");
                    auto* elem = builder_->CreateLoad(elemTy, gep, "elem");
                    llvm::Value* eq;
                    if (elemTy->isIntegerTy())
                        eq = builder_->CreateICmpEQ(elem, needle, "eq");
                    else if (elemTy->isFloatingPointTy())
                        eq = builder_->CreateFCmpOEQ(elem, needle, "eq");
                    else
                        eq = builder_->CreateICmpEQ(elem, needle, "eq");
                    builder_->CreateCondBr(eq, foundBB, nextBB);

                    builder_->SetInsertPoint(foundBB);
                    builder_->CreateStore(builder_->CreateLoad(i64Ty, idxAlloca, "idx"), resultAlloca);
                    builder_->CreateBr(endBB);

                    builder_->SetInsertPoint(nextBB);
                    auto* nextIdx = builder_->CreateAdd(
                        builder_->CreateLoad(i64Ty, idxAlloca, "idx"),
                        llvm::ConstantInt::get(i64Ty, 1), "next");
                    builder_->CreateStore(nextIdx, idxAlloca);
                    builder_->CreateBr(condBB);

                    builder_->SetInsertPoint(endBB);
                } else {
                    // Backward scan: len-1..0, break on first match
                    auto* condBB = llvm::BasicBlock::Create(*context_, "lastIdx.cond", currentFunction_);
                    auto* bodyBB = llvm::BasicBlock::Create(*context_, "lastIdx.body", currentFunction_);
                    auto* foundBB = llvm::BasicBlock::Create(*context_, "lastIdx.found", currentFunction_);
                    auto* nextBB = llvm::BasicBlock::Create(*context_, "lastIdx.next", currentFunction_);
                    auto* endBB  = llvm::BasicBlock::Create(*context_, "lastIdx.end", currentFunction_);

                    auto* idxAlloca = builder_->CreateAlloca(i64Ty, nullptr, "idx");
                    builder_->CreateStore(llvm::ConstantInt::get(i64Ty, arrLen), idxAlloca);
                    builder_->CreateBr(condBB);

                    builder_->SetInsertPoint(condBB);
                    auto* idx = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
                    auto* cmp = builder_->CreateICmpUGT(idx, llvm::ConstantInt::get(i64Ty, 0), "cmp");
                    builder_->CreateCondBr(cmp, bodyBB, endBB);

                    builder_->SetInsertPoint(bodyBB);
                    auto* decIdx = builder_->CreateSub(
                        builder_->CreateLoad(i64Ty, idxAlloca, "idx"),
                        llvm::ConstantInt::get(i64Ty, 1), "dec");
                    builder_->CreateStore(decIdx, idxAlloca);
                    auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                    auto* gep = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, decIdx}, "elem_ptr");
                    auto* elem = builder_->CreateLoad(elemTy, gep, "elem");
                    llvm::Value* eq;
                    if (elemTy->isIntegerTy())
                        eq = builder_->CreateICmpEQ(elem, needle, "eq");
                    else if (elemTy->isFloatingPointTy())
                        eq = builder_->CreateFCmpOEQ(elem, needle, "eq");
                    else
                        eq = builder_->CreateICmpEQ(elem, needle, "eq");
                    builder_->CreateCondBr(eq, foundBB, nextBB);

                    builder_->SetInsertPoint(foundBB);
                    builder_->CreateStore(builder_->CreateLoad(i64Ty, idxAlloca, "idx"), resultAlloca);
                    builder_->CreateBr(endBB);

                    builder_->SetInsertPoint(nextBB);
                    builder_->CreateBr(condBB);

                    builder_->SetInsertPoint(endBB);
                }
                return static_cast<llvm::Value*>(
                    builder_->CreateLoad(i64Ty, resultAlloca, methodName));
            }
            if (methodName == "count") {
                if (args.empty()) {
                    std::cerr << "lux: count() requires a value argument\n";
                    return static_cast<llvm::Value*>(llvm::ConstantInt::get(usizeTy, 0));
                }
                auto* needle = castToElem(args[0]);
                auto* countAlloca = builder_->CreateAlloca(usizeTy, nullptr, "count_res");
                builder_->CreateStore(llvm::ConstantInt::get(usizeTy, 0), countAlloca);

                auto* condBB = llvm::BasicBlock::Create(*context_, "count.cond", currentFunction_);
                auto* bodyBB = llvm::BasicBlock::Create(*context_, "count.body", currentFunction_);
                auto* incBB  = llvm::BasicBlock::Create(*context_, "count.inc", currentFunction_);
                auto* nextBB = llvm::BasicBlock::Create(*context_, "count.next", currentFunction_);
                auto* endBB  = llvm::BasicBlock::Create(*context_, "count.end", currentFunction_);

                auto* idxAlloca = builder_->CreateAlloca(i64Ty, nullptr, "idx");
                builder_->CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);
                builder_->CreateBr(condBB);

                builder_->SetInsertPoint(condBB);
                auto* idx = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
                auto* cmp = builder_->CreateICmpULT(idx, llvm::ConstantInt::get(i64Ty, arrLen), "cmp");
                builder_->CreateCondBr(cmp, bodyBB, endBB);

                builder_->SetInsertPoint(bodyBB);
                auto* curIdx = builder_->CreateLoad(i64Ty, idxAlloca, "idx");
                auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                auto* gep = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, curIdx}, "elem_ptr");
                auto* elem = builder_->CreateLoad(elemTy, gep, "elem");
                llvm::Value* eq;
                if (elemTy->isIntegerTy())
                    eq = builder_->CreateICmpEQ(elem, needle, "eq");
                else if (elemTy->isFloatingPointTy())
                    eq = builder_->CreateFCmpOEQ(elem, needle, "eq");
                else
                    eq = builder_->CreateICmpEQ(elem, needle, "eq");
                builder_->CreateCondBr(eq, incBB, nextBB);

                builder_->SetInsertPoint(incBB);
                auto* cur = builder_->CreateLoad(usizeTy, countAlloca, "cur");
                builder_->CreateStore(
                    builder_->CreateAdd(cur, llvm::ConstantInt::get(usizeTy, 1), "inc"),
                    countAlloca);
                builder_->CreateBr(nextBB);

                builder_->SetInsertPoint(nextBB);
                auto* nextIdx = builder_->CreateAdd(
                    builder_->CreateLoad(i64Ty, idxAlloca, "idx"),
                    llvm::ConstantInt::get(i64Ty, 1), "next");
                builder_->CreateStore(nextIdx, idxAlloca);
                builder_->CreateBr(condBB);

                builder_->SetInsertPoint(endBB);
                return static_cast<llvm::Value*>(
                    builder_->CreateLoad(usizeTy, countAlloca, "count"));
            }
            if (methodName == "fill") {
                if (args.empty()) {
                    std::cerr << "lux: fill() requires a value argument\n";
                    return static_cast<llvm::Value*>(
                        llvm::UndefValue::get(llvm::Type::getVoidTy(*context_)));
                }
                auto* fillVal = castToElem(args[0]);
                for (uint64_t i = 0; i < arrLen; i++) {
                    auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                    auto* idx  = llvm::ConstantInt::get(i64Ty, i);
                    auto* gep  = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, idx}, "fill_ptr");
                    builder_->CreateStore(fillVal, gep);
                }
                return static_cast<llvm::Value*>(
                    llvm::UndefValue::get(llvm::Type::getVoidTy(*context_)));
            }
            if (methodName == "swap") {
                if (args.size() < 2) {
                    std::cerr << "lux: swap() requires two index arguments\n";
                    return static_cast<llvm::Value*>(
                        llvm::UndefValue::get(llvm::Type::getVoidTy(*context_)));
                }
                auto* idxA = args[0];
                auto* idxB = args[1];
                if (idxA->getType() != i64Ty) idxA = builder_->CreateIntCast(idxA, i64Ty, true);
                if (idxB->getType() != i64Ty) idxB = builder_->CreateIntCast(idxB, i64Ty, true);
                auto* zero = llvm::ConstantInt::get(i64Ty, 0);
                auto* gepA = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, idxA}, "swap_a");
                auto* gepB = builder_->CreateInBoundsGEP(arrTy, arrAlloca, {zero, idxB}, "swap_b");
                auto* valA = builder_->CreateLoad(elemTy, gepA, "a");
                auto* valB = builder_->CreateLoad(elemTy, gepB, "b");
                builder_->CreateStore(valB, gepA);
                builder_->CreateStore(valA, gepB);
                return static_cast<llvm::Value*>(
                    llvm::UndefValue::get(llvm::Type::getVoidTy(*context_)));
            }
        }
    }

    // Fallback: return undef of the expected return type
    const TypeInfo* retTI = nullptr;
    if (desc->returnType == "_self")
        retTI = receiverTI;
    else if (desc->returnType == "_elem")
        retTI = receiverTI;
    else
        retTI = typeRegistry_.lookup(desc->returnType);

    llvm::Type* retTy = retTI
        ? retTI->toLLVMType(*context_, module_->getDataLayout())
        : llvm::Type::getVoidTy(*context_);

    if (retTy->isVoidTy())
        return static_cast<llvm::Value*>(
            llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));

    return static_cast<llvm::Value*>(llvm::UndefValue::get(retTy));
}

// ── spawn expression ────────────────────────────────────────────────────
// spawn funcName(args) → Task<T> (returns ptr to lux_Task)
//
// Implementation:
//   1. Pack arguments into a heap-allocated struct
//   2. Create a trampoline function: void* __spawn_N(void* packed) that
//      unpacks args, calls the target, and returns heap-allocated result
//   3. Call lux_taskCreate(trampoline, packed) → Task*
std::any IRGen::visitSpawnExpr(LuxParser::SpawnExprContext* ctx) {
    auto* innerExpr = ctx->expression();

    // The inner expression must be a function call
    auto* fnCall = dynamic_cast<LuxParser::FnCallExprContext*>(innerExpr);
    if (!fnCall) {
        std::cerr << "lux: spawn requires a function call expression\n";
        return static_cast<llvm::Value*>(
            llvm::ConstantPointerNull::get(
                llvm::PointerType::getUnqual(*context_)));
    }

    // The fnCallExpr has expression()(argList?) — the callee is the inner expression
    auto* calleeExpr = fnCall->expression();
    auto* identExpr = dynamic_cast<LuxParser::IdentExprContext*>(calleeExpr);
    if (!identExpr) {
        std::cerr << "lux: spawn requires a direct function call (e.g. spawn func())\n";
        return static_cast<llvm::Value*>(
            llvm::ConstantPointerNull::get(
                llvm::PointerType::getUnqual(*context_)));
    }

    std::string targetName = identExpr->IDENTIFIER()->getText();

    // Evaluate arguments
    std::vector<llvm::Value*> args;
    if (auto* argList = fnCall->argList()) {
        for (auto* e : argList->expression())
            args.push_back(std::any_cast<llvm::Value*>(visit(e)));
    }

    // Look up the target function
    auto* targetFn = module_->getFunction(targetName);
    if (!targetFn) {
        std::cerr << "lux: spawn target function '" << targetName << "' not found\n";
        return static_cast<llvm::Value*>(
            llvm::ConstantPointerNull::get(
                llvm::PointerType::getUnqual(*context_)));
    }

    auto* ptrTy = llvm::PointerType::getUnqual(*context_);
    auto* i8Ty  = llvm::Type::getInt8Ty(*context_);

    // ── Step 1: Pack arguments into a heap struct ──────────────────────
    llvm::Value* packedPtr = llvm::ConstantPointerNull::get(ptrTy);
    llvm::StructType* packedTy = nullptr;

    if (!args.empty()) {
        // Cast arguments to match target function parameter types
        auto* fnType = targetFn->getFunctionType();
        for (size_t i = 0; i < args.size() && i < fnType->getNumParams(); i++) {
            auto* paramTy = fnType->getParamType(i);
            if (args[i]->getType() != paramTy) {
                if (args[i]->getType()->isIntegerTy() && paramTy->isIntegerTy()) {
                    args[i] = builder_->CreateIntCast(args[i], paramTy, true);
                } else if (args[i]->getType()->isFloatingPointTy() && paramTy->isFloatingPointTy()) {
                    args[i] = builder_->CreateFPCast(args[i], paramTy);
                } else if (args[i]->getType()->isIntegerTy() && paramTy->isFloatingPointTy()) {
                    args[i] = builder_->CreateSIToFP(args[i], paramTy);
                } else if (args[i]->getType()->isFloatingPointTy() && paramTy->isIntegerTy()) {
                    args[i] = builder_->CreateFPToSI(args[i], paramTy);
                }
            }
        }

        std::vector<llvm::Type*> fieldTypes;
        for (auto* a : args)
            fieldTypes.push_back(a->getType());

        packedTy = llvm::StructType::create(*context_, fieldTypes,
            "__spawn_args_" + std::to_string(spawnCounter_));

        // malloc(sizeof(packedTy))
        auto* dlayout = &module_->getDataLayout();
        auto* allocSize = llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(*context_),
            dlayout->getTypeAllocSize(packedTy));
        auto mallocFn = declareBuiltin("malloc", ptrTy,
            {llvm::Type::getInt64Ty(*context_)});
        packedPtr = builder_->CreateCall(mallocFn, {allocSize}, "spawn_args");

        // Store each arg
        for (size_t i = 0; i < args.size(); i++) {
            auto* gep = builder_->CreateStructGEP(packedTy, packedPtr, i);
            builder_->CreateStore(args[i], gep);
        }
    }

    // ── Step 2: Create trampoline function ─────────────────────────────
    auto trampolineName = "__spawn_trampoline_" + std::to_string(spawnCounter_++);
    auto* trampolineType = llvm::FunctionType::get(ptrTy, {ptrTy}, false);
    auto* trampolineFn = llvm::Function::Create(
        trampolineType, llvm::Function::InternalLinkage,
        trampolineName, module_);

    // Save current state
    auto* savedBB   = builder_->GetInsertBlock();
    auto* savedFunc = currentFunction_;
    auto  savedLocals = locals_;

    // Build trampoline body
    auto* trampEntry = llvm::BasicBlock::Create(*context_, "entry", trampolineFn);
    builder_->SetInsertPoint(trampEntry);
    currentFunction_ = trampolineFn;

    auto* argPtr = trampolineFn->getArg(0);
    argPtr->setName("packed");

    // Unpack arguments
    std::vector<llvm::Value*> unpackedArgs;
    if (packedTy) {
        for (size_t i = 0; i < args.size(); i++) {
            auto* gep = builder_->CreateStructGEP(packedTy, argPtr, i);
            auto* val = builder_->CreateLoad(args[i]->getType(), gep);
            unpackedArgs.push_back(val);
        }
        // Free the packed args struct
        auto freeFn = declareBuiltin("free",
            llvm::Type::getVoidTy(*context_), {ptrTy});
        builder_->CreateCall(freeFn, {argPtr});
    }

    // Call the target function
    auto* retTy = targetFn->getReturnType();
    llvm::Value* result;
    if (retTy->isVoidTy()) {
        builder_->CreateCall(targetFn, unpackedArgs);
        result = llvm::ConstantPointerNull::get(ptrTy);
    } else {
        auto* callResult = builder_->CreateCall(targetFn, unpackedArgs, "spawn_result");

        // Heap-allocate the result
        auto* resultSize = llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(*context_),
            module_->getDataLayout().getTypeAllocSize(retTy));
        auto mallocFn = declareBuiltin("malloc", ptrTy,
            {llvm::Type::getInt64Ty(*context_)});
        auto* heapResult = builder_->CreateCall(mallocFn, {resultSize}, "heap_result");
        builder_->CreateStore(callResult, heapResult);
        result = heapResult;
    }

    builder_->CreateRet(result);

    // ── Restore caller state ───────────────────────────────────────────
    currentFunction_ = savedFunc;
    locals_ = savedLocals;
    builder_->SetInsertPoint(savedBB);

    // ── Step 3: Call lux_taskCreate(trampoline, packed) ──────────────
    auto taskCreateFn = declareBuiltin("lux_taskCreate", ptrTy,
        {ptrTy, ptrTy});
    auto* task = builder_->CreateCall(taskCreateFn,
        {trampolineFn, packedPtr}, "task");

    return static_cast<llvm::Value*>(task);
}

// ── await expression ────────────────────────────────────────────────────
// await task → T (extracts result from Task<T>)
std::any IRGen::visitAwaitExpr(LuxParser::AwaitExprContext* ctx) {
    auto* taskVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    auto* ptrTy = llvm::PointerType::getUnqual(*context_);

    // Load the task handle if it's stored in an alloca
    if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(taskVal)) {
        taskVal = builder_->CreateLoad(ptrTy, alloca, "task_ptr");
    }

    // Call lux_taskAwait(task) → void* (result pointer)
    auto awaitFn = declareBuiltin("lux_taskAwait", ptrTy, {ptrTy});
    auto* resultPtr = builder_->CreateCall(awaitFn, {taskVal}, "await_result");

    // Resolve the Task<T> element type to know what to load from result
    const TypeInfo* taskTI = nullptr;
    if (auto* identExpr = dynamic_cast<LuxParser::IdentExprContext*>(
            ctx->expression())) {
        auto it = locals_.find(identExpr->IDENTIFIER()->getText());
        if (it != locals_.end())
            taskTI = it->second.typeInfo;
    }

    // If Task<T> with T != void, load the result
    if (taskTI && taskTI->elementType &&
        taskTI->elementType->kind != TypeKind::Void) {
        auto* resultTy = taskTI->elementType->toLLVMType(
            *context_, module_->getDataLayout());
        auto* result = builder_->CreateLoad(resultTy, resultPtr, "task_value");

        // Free the heap-allocated result and the task handle
        auto freeFn = declareBuiltin("free",
            llvm::Type::getVoidTy(*context_), {ptrTy});
        builder_->CreateCall(freeFn, {resultPtr});

        auto taskFreeFn = declareBuiltin("lux_taskFree",
            llvm::Type::getVoidTy(*context_), {ptrTy});
        builder_->CreateCall(taskFreeFn, {taskVal});

        return static_cast<llvm::Value*>(result);
    }

    // Void task — just await and free
    auto taskFreeFn = declareBuiltin("lux_taskFree",
        llvm::Type::getVoidTy(*context_), {ptrTy});
    builder_->CreateCall(taskFreeFn, {taskVal});

    return static_cast<llvm::Value*>(
        llvm::UndefValue::get(llvm::Type::getInt32Ty(*context_)));
}

// ── lock statement ──────────────────────────────────────────────────────
// lock(mtx) { ... } → lock, execute block, unlock (even on early return)
std::any IRGen::visitLockStmt(LuxParser::LockStmtContext* ctx) {
    auto* mtxVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    auto* ptrTy = llvm::PointerType::getUnqual(*context_);

    // Load mutex handle from alloca
    if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(mtxVal)) {
        mtxVal = builder_->CreateLoad(ptrTy, alloca, "mtx_ptr");
    }

    // Lock
    auto lockFn = declareBuiltin("lux_mutexLock",
        llvm::Type::getVoidTy(*context_), {ptrTy});
    builder_->CreateCall(lockFn, {mtxVal});

    // Execute block body
    for (auto* stmt : ctx->block()->statement()) {
        visit(stmt);
    }

    // Unlock
    auto unlockFn = declareBuiltin("lux_mutexUnlock",
        llvm::Type::getVoidTy(*context_), {ptrTy});
    builder_->CreateCall(unlockFn, {mtxVal});

    return {};
}

// ── try/catch/finally ────────────────────────────────────────────────────────

std::any IRGen::visitTryCatchStmt(LuxParser::TryCatchStmtContext* ctx) {
    auto* fn     = currentFunction_;
    auto* ptrTy  = llvm::PointerType::getUnqual(*context_);
    auto* i32Ty  = llvm::Type::getInt32Ty(*context_);
    auto* voidTy = llvm::Type::getVoidTy(*context_);
    auto* errorTy = llvm::StructType::getTypeByName(*context_, "Error");

    // Allocate eh_frame on the heap (opaque, platform-independent size)
    auto allocFn = declareBuiltin("lux_eh_alloc", ptrTy, {});
    auto* framePtr = builder_->CreateCall(allocFn, {}, "eh_frame");

    // Push frame onto exception handler stack
    auto pushFn = declareBuiltin("lux_eh_push", voidTy, {ptrTy});
    builder_->CreateCall(pushFn, {framePtr});

    // Get jmp_buf pointer from frame and call setjmp
    auto getJmpBufFn = declareBuiltin("lux_eh_get_jmpbuf", ptrTy, {ptrTy});
    auto* jmpBufPtr  = builder_->CreateCall(getJmpBufFn, {framePtr}, "jmpbuf_ptr");
    auto setjmpFn    = declareBuiltin("setjmp", i32Ty, {ptrTy});
    auto* sjResult   = builder_->CreateCall(setjmpFn, {jmpBufPtr});

    auto* isCaught = builder_->CreateICmpNE(sjResult,
        llvm::ConstantInt::get(i32Ty, 0), "is_caught");

    // Basic blocks
    auto* tryBB     = llvm::BasicBlock::Create(*context_, "try",     fn);
    auto* catchBB   = llvm::BasicBlock::Create(*context_, "catch",   fn);
    auto* finallyBB = llvm::BasicBlock::Create(*context_, "finally", fn);
    auto* mergeBB   = llvm::BasicBlock::Create(*context_, "try_end", fn);

    builder_->CreateCondBr(isCaught, catchBB, tryBB);

    // ── try block ───────────────────────────────────────────────────────
    builder_->SetInsertPoint(tryBB);
    for (auto* stmt : ctx->block()->statement())
        visit(stmt);

    // Pop handler after successful try
    auto popFn = declareBuiltin("lux_eh_pop", voidTy, {});
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateCall(popFn, {});
        builder_->CreateBr(finallyBB);
    }

    // ── catch block(s) ──────────────────────────────────────────────────
    builder_->SetInsertPoint(catchBB);

    // Pop handler before running catch body
    builder_->CreateCall(popFn, {});

    // Load error from frame via helper
    auto getErrFn = declareBuiltin("lux_eh_get_error", ptrTy, {ptrTy});
    auto* errorPtr = builder_->CreateCall(getErrFn, {framePtr}, "error_ptr");
    auto* errorVal = builder_->CreateLoad(errorTy, errorPtr, "caught_error");

    // For each catch clause, introduce the error variable
    for (auto* catchCtx : ctx->catchClause()) {
        auto varName = catchCtx->IDENTIFIER()->getText();

        auto* errAlloca = builder_->CreateAlloca(errorTy, nullptr, varName);
        builder_->CreateStore(errorVal, errAlloca);

        auto* errTI = typeRegistry_.lookup("Error");
        auto saved = locals_[varName];
        locals_[varName] = { errAlloca, errTI, 0 };

        for (auto* stmt : catchCtx->block()->statement()) {
            visit(stmt);
        }

        // Restore previous binding if there was one
        if (saved.alloca)
            locals_[varName] = saved;
        else
            locals_.erase(varName);
    }

    if (!builder_->GetInsertBlock()->getTerminator())
        builder_->CreateBr(finallyBB);

    // ── finally block ───────────────────────────────────────────────────
    builder_->SetInsertPoint(finallyBB);

    if (ctx->finallyClause()) {
        for (auto* stmt : ctx->finallyClause()->block()->statement()) {
            visit(stmt);
        }
    }

    // Free the heap-allocated frame
    auto freeFn = declareBuiltin("lux_eh_free", voidTy, {ptrTy});
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateCall(freeFn, {framePtr});
        builder_->CreateBr(mergeBB);
    }

    builder_->SetInsertPoint(mergeBB);
    return {};
}

std::any IRGen::visitThrowStmt(LuxParser::ThrowStmtContext* ctx) {
    auto* ptrTy  = llvm::PointerType::getUnqual(*context_);
    auto* voidTy = llvm::Type::getVoidTy(*context_);
    auto* usizeTy = module_->getDataLayout().getIntPtrType(*context_);
    auto* i32Ty  = llvm::Type::getInt32Ty(*context_);
    auto* errorTy = llvm::StructType::getTypeByName(*context_, "Error");

    // Get source location from the throw statement token
    auto* throwToken = ctx->THROW()->getSymbol();
    unsigned srcLine = throwToken->getLine();
    unsigned srcCol  = throwToken->getCharPositionInLine();

    // Get the source file name
    std::string fileName = module_->getSourceFileName();

    // Evaluate the expression (expected to be Error struct literal)
    auto* errorVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));

    // Error struct layout: { {ptr,usize} message, {ptr,usize} file, i32 line, i32 column }
    //                        field 0               field 1           field 2    field 3

    // Auto-fill file if empty (len == 0)
    auto* fileLen = builder_->CreateExtractValue(errorVal, {1, 1}, "file_len");
    auto* fileIsEmpty = builder_->CreateICmpEQ(fileLen,
        llvm::ConstantInt::get(usizeTy, 0), "file_empty");

    auto* fileGlobal = builder_->CreateGlobalString(fileName, ".throw_file", 0, module_);
    auto* fileLenVal = llvm::ConstantInt::get(usizeTy, fileName.size());

    auto* curFilePtr = builder_->CreateExtractValue(errorVal, {1, 0});
    auto* filePtr    = builder_->CreateSelect(fileIsEmpty, fileGlobal, curFilePtr);
    auto* fileLenFin = builder_->CreateSelect(fileIsEmpty, fileLenVal, fileLen);
    errorVal = builder_->CreateInsertValue(errorVal, filePtr,    {1, 0});
    errorVal = builder_->CreateInsertValue(errorVal, fileLenFin, {1, 1});

    // Auto-fill line if zero
    auto* lineVal    = builder_->CreateExtractValue(errorVal, {2}, "line_val");
    auto* lineIsZero = builder_->CreateICmpEQ(lineVal, llvm::ConstantInt::get(i32Ty, 0));
    auto* lineFinal  = builder_->CreateSelect(lineIsZero,
        llvm::ConstantInt::get(i32Ty, srcLine), lineVal);
    errorVal = builder_->CreateInsertValue(errorVal, lineFinal, {2});

    // Auto-fill column if zero
    auto* colVal    = builder_->CreateExtractValue(errorVal, {3}, "col_val");
    auto* colIsZero = builder_->CreateICmpEQ(colVal, llvm::ConstantInt::get(i32Ty, 0));
    auto* colFinal  = builder_->CreateSelect(colIsZero,
        llvm::ConstantInt::get(i32Ty, srcCol), colVal);
    errorVal = builder_->CreateInsertValue(errorVal, colFinal, {3});

    // Store to temp alloca and pass by pointer to lux_eh_throw
    auto* errAlloca = builder_->CreateAlloca(errorTy, nullptr, "throw_err");
    builder_->CreateStore(errorVal, errAlloca);

    auto throwFn = declareBuiltin("lux_eh_throw", voidTy, {ptrTy});
    builder_->CreateCall(throwFn, {errAlloca});

    builder_->CreateUnreachable();

    // Dead block for any subsequent unreachable statements
    auto* deadBB = llvm::BasicBlock::Create(*context_, "throw.dead", currentFunction_);
    builder_->SetInsertPoint(deadBB);

    return {};
}

std::any IRGen::visitTryExpr(LuxParser::TryExprContext* ctx) {
    auto* fn     = currentFunction_;
    auto* ptrTy  = llvm::PointerType::getUnqual(*context_);
    auto* i32Ty  = llvm::Type::getInt32Ty(*context_);
    auto* voidTy = llvm::Type::getVoidTy(*context_);

    // Allocate eh_frame on the heap
    auto allocFn = declareBuiltin("lux_eh_alloc", ptrTy, {});
    auto* framePtr = builder_->CreateCall(allocFn, {}, "try_frame");

    // Push frame onto exception handler stack
    auto pushFn = declareBuiltin("lux_eh_push", voidTy, {ptrTy});
    builder_->CreateCall(pushFn, {framePtr});

    // Get jmp_buf pointer and call setjmp
    auto getJmpBufFn = declareBuiltin("lux_eh_get_jmpbuf", ptrTy, {ptrTy});
    auto* jmpBufPtr  = builder_->CreateCall(getJmpBufFn, {framePtr}, "try_jmpbuf");
    auto setjmpFn    = declareBuiltin("setjmp", i32Ty, {ptrTy});
    auto* sjResult   = builder_->CreateCall(setjmpFn, {jmpBufPtr});

    auto* isCaught = builder_->CreateICmpNE(sjResult,
        llvm::ConstantInt::get(i32Ty, 0), "try_caught");

    auto* exprBB    = llvm::BasicBlock::Create(*context_, "try.expr",    fn);
    auto* defaultBB = llvm::BasicBlock::Create(*context_, "try.default", fn);
    auto* mergeBB   = llvm::BasicBlock::Create(*context_, "try.merge",   fn);

    builder_->CreateCondBr(isCaught, defaultBB, exprBB);

    // ── Expression evaluation (no exception) ────────────────────────────
    builder_->SetInsertPoint(exprBB);

    auto popFn  = declareBuiltin("lux_eh_pop",  voidTy, {});
    auto freeFn = declareBuiltin("lux_eh_free", voidTy, {ptrTy});

    auto* exprVal = std::any_cast<llvm::Value*>(visit(ctx->expression()));
    auto* exprTy  = exprVal->getType();

    builder_->CreateCall(popFn, {});
    builder_->CreateCall(freeFn, {framePtr});

    auto* exprEndBB = builder_->GetInsertBlock();
    builder_->CreateBr(mergeBB);

    // ── Default value (exception caught) ────────────────────────────────
    builder_->SetInsertPoint(defaultBB);

    builder_->CreateCall(popFn, {});
    builder_->CreateCall(freeFn, {framePtr});

    llvm::Value* defaultVal = llvm::Constant::getNullValue(exprTy);

    builder_->CreateBr(mergeBB);

    // ── Merge ───────────────────────────────────────────────────────────
    builder_->SetInsertPoint(mergeBB);

    auto* phi = builder_->CreatePHI(exprTy, 2, "try_result");
    phi->addIncoming(exprVal, exprEndBB);
    phi->addIncoming(defaultVal, defaultBB);

    return static_cast<llvm::Value*>(phi);
}

std::pair<llvm::Value*, llvm::Value*>
IRGen::promoteArithmetic(llvm::Value* lhs, llvm::Value* rhs) {
    auto* lhsTy = lhs->getType();
    auto* rhsTy = rhs->getType();
    if (lhsTy == rhsTy) return {lhs, rhs};

    // Both integer: extend narrower to wider
    if (lhsTy->isIntegerTy() && rhsTy->isIntegerTy()) {
        if (lhsTy->getIntegerBitWidth() < rhsTy->getIntegerBitWidth())
            lhs = builder_->CreateSExt(lhs, rhsTy, "prom");
        else
            rhs = builder_->CreateSExt(rhs, lhsTy, "prom");
        return {lhs, rhs};
    }

    // Both float: extend narrower to wider
    if (lhsTy->isFloatingPointTy() && rhsTy->isFloatingPointTy()) {
        if (lhsTy->getPrimitiveSizeInBits() < rhsTy->getPrimitiveSizeInBits())
            lhs = builder_->CreateFPExt(lhs, rhsTy, "prom");
        else
            rhs = builder_->CreateFPExt(rhs, lhsTy, "prom");
        return {lhs, rhs};
    }

    return {lhs, rhs};
}
