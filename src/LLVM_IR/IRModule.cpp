#include "LLVM_IR/IRModule.h"

#include <llvm/Support/raw_ostream.h>

IRModule::IRModule(std::unique_ptr<llvm::LLVMContext> context,
                   std::unique_ptr<llvm::Module>      module)
    : context_(std::move(context)), module_(std::move(module)) {}

void IRModule::print() const {
    module_->print(llvm::outs(), nullptr);
}

std::string IRModule::toString() const {
    std::string output;
    llvm::raw_string_ostream stream(output);
    module_->print(stream, nullptr);
    return stream.str();
}
