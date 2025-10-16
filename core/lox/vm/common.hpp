#ifndef LOX_VM_COMMON_HPP
#define LOX_VM_COMMON_HPP

#include <cstdint>

namespace lox::vm {

    /**
     * @enum OpCode
     * @brief The operation codes for the bytecode instructions.
     */
    enum class OpCode : std::uint8_t {
        OP_CONSTANT,
        OP_ADD,
        OP_SUBTRACT,
        OP_MULTIPLY,
        OP_DIVIDE,
        OP_NEGATE,
        OP_RETURN,
    };

} // namespace lox::vm

#endif
