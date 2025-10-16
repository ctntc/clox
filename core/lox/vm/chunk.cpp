#include "chunk.hpp"

#include "common.hpp"

#include <cstdint>
#include <print>

namespace lox::vm {

    Chunk::Chunk() : count(0), capacity(0), lines(0), code({}), constants({}) {}

    auto Chunk::write(uint8_t byte, size_t line) noexcept -> void {
        code.push_back(byte);
        lines.push_back(line);
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

        if (offset > 0 && lines[offset] == lines[offset - 1]) {
            std::print("    | ");
        } else {
            std::print("{:4} ", lines[offset]);
        }

        const auto instruction = static_cast<OpCode>(code[offset]);
        switch (instruction) {
        case OpCode::OP_CONSTANT:
            return disassemble_constant_instruction("OP_CONSTANT", offset);
        case OpCode::OP_ADD:
            std::println("OP_ADD");
            return offset + 1;
        case OpCode::OP_SUBTRACT:
            std::println("OP_SUBTRACT");
            return offset + 1;
        case OpCode::OP_MULTIPLY:
            std::println("OP_MULTIPLY");
            return offset + 1;
        case OpCode::OP_DIVIDE:
            std::println("OP_DIVIDE");
            return offset + 1;
        case OpCode::OP_NEGATE:
            std::println("OP_NEGATE");
            return offset + 1;
        case OpCode::OP_RETURN:
            std::println("OP_RETURN");
            return offset + 1;
        default:
            std::println("Unknown opcode {}", static_cast<int>(code[offset]));
            return offset + 1;
        }
    }

    auto Chunk::disassemble_constant_instruction(const std::string_view name, size_t offset) noexcept -> size_t {
        const auto constant_index = code[offset + 1];
        std::println("{} {:4} '{}'", name, constant_index, static_cast<double>(constants[constant_index]));
        return offset + 2;
    }

} // namespace lox::vm
