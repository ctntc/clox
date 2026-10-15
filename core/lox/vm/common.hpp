#ifndef LOX_VM_COMMON_HPP
#define LOX_VM_COMMON_HPP

#include <cstdint>

namespace lox::vm {

    /**
     * @enum OpCode
     * @brief The operation codes for the bytecode instructions.
     */
    enum class OpCode : std::uint8_t {
        constant,
        add,
        subtract,
        multiply,
        divide,
        negate,
        ret,
    };

} // namespace lox::vm

#endif
