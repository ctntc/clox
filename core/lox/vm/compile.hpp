#ifndef LOX_VM_COMPILE_HPP
#define LOX_VM_COMPILE_HPP

#include "lox/vm/chunk.hpp"

#include <string_view>

namespace lox::vm {

    class Compiler final {
    public:
        explicit Compiler() = default;

        auto compile(std::string_view source, Chunk *chunk) noexcept -> bool;

    private:
        std::string_view m_source;
    };

} // namespace lox::vm

#endif
