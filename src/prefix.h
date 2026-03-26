// Force-include standard headers before LLVM / ANTLR4 to work around
// libstdc++ 15 header-ordering issues with Clang (std::max overload
// resolution in constexpr context).
#include <cstddef>
#include <algorithm>
#include <initializer_list>
