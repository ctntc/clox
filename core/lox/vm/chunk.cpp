#include "chunk.hpp"

#include "common.hpp"

#include <cstdint>
#include <print>

namespace lox::vm {

    Chunk::Chunk() : count(0), capacity(0), code({}), constants({}) {}

    auto Chunk::write(uint8_t byte) noexcept -> void {
        code.push_back(byte);
        count++;
    }

    auto Chunk::add_constant(Value value) noexcept -> size_t {
        constants.push_back(value);
        return constants.size() - 1;
    }

    auto Chunk::disassemble(std::string_view name) noexcept -> void {
        std::println("== {} ==\n", name);

        for (size_t offset = 0; offset < count;) {
            offset = this->disassemble_instruction(offset);
        }
    }

    auto Chunk::disassemble_instruction(size_t offset) noexcept -> size_t {
        std::print("{:04} ", offset);

        const auto instruction = static_cast<OpCode>(code[offset]);
        switch (instruction) {
        case OpCode::OP_RETURN:
            std::println("OP_RETURN");
            return offset + 1;
        default:
            std::println("Unknown opcode {}", static_cast<int>(code[offset]));
            return offset + 1;
        }
    }

} // namespace lox::vm
