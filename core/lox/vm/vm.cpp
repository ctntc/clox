#include "lox/vm/vm.hpp"

#include "lox/vm/chunk.hpp"
#include "lox/vm/common.hpp"
#include "lox/vm/compile.hpp"
#include "lox/vm/value.hpp"

#include <spdlog/spdlog.h>

#include <cstdint>
#include <functional>
#include <print>
#include <string_view>
#include <utility>

namespace lox::vm {

    auto interpret_result_to_string(InterpretResult result) noexcept -> std::string_view {
        switch (result) {
        case InterpretResult::ok:
            return "OK";
        case InterpretResult::compile_error:
            return "Compile Error";
        case InterpretResult::runtime_error:
            return "Runtime Error";
        default:
            return "Unknown Error";
        }
    }

    VirtualMachine::VirtualMachine() : m_stack(), m_chunks() { m_stack.reserve(STACK_MAX); }

    auto VirtualMachine::interpret(std::string_view source) noexcept -> InterpretResult {
        Chunk chunk;
        Compiler compiler;
        if (!compiler.compile(source, &chunk)) {
            return InterpretResult::compile_error;
        }

        m_chunks.push_back(std::move(chunk));
        m_chunk = &m_chunks.back();
        m_ip = 0;
        return InterpretResult::ok;
        // return run();
    }

    auto VirtualMachine::run() noexcept -> InterpretResult {
        while (true) {
#ifndef NDEBUG
            std::print("          ");
            for (const auto &slot : m_stack) {
                std::print("[ {} ]", slot);
            }
            std::println();
            m_chunk->disassemble_instruction(m_ip);
#endif
            uint8_t instruction = read_byte();
            switch (instruction) {
            case static_cast<uint8_t>(OpCode::constant): {
                Value constant = read_constant();
                m_stack.push_back(constant);
            } break;
            case static_cast<uint8_t>(OpCode::add): {
                if (!perform_binary_operation(std::plus<>{})) {
                    return InterpretResult::runtime_error;
                }
            } break;
            case static_cast<uint8_t>(OpCode::subtract): {
                if (!perform_binary_operation(std::minus<>{})) {
                    return InterpretResult::runtime_error;
                }
            } break;
            case static_cast<uint8_t>(OpCode::multiply): {
                if (!perform_binary_operation(std::multiplies<>{})) {
                    return InterpretResult::runtime_error;
                }
            } break;
            case static_cast<uint8_t>(OpCode::divide): {
                if (!perform_binary_operation(std::divides<>{})) {
                    return InterpretResult::runtime_error;
                }
            } break;
            case static_cast<uint8_t>(OpCode::negate): {
                if (m_stack.empty()) {
                    spdlog::error("Stack underflow");
                    return InterpretResult::runtime_error;
                }
                m_stack.back() = -m_stack.back();
            } break;
            case static_cast<uint8_t>(OpCode::ret): {
                if (m_stack.empty()) {
                    spdlog::error("Stack underflow");
                    return InterpretResult::runtime_error;
                }
                std::println("{}", m_stack.back());
                return InterpretResult::ok;
            }
            default:
                spdlog::error("Unknown opcode {}", instruction);
                return InterpretResult::runtime_error;
            }
        }
    }

    // Hot path: unchecked indexing. The compiler is responsible for emitting valid bytecode.
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    [[nodiscard]] auto VirtualMachine::read_byte() noexcept -> uint8_t { return m_chunk->code[m_ip++]; }
    [[nodiscard]] auto VirtualMachine::read_constant() noexcept -> Value { return m_chunk->constants[read_byte()]; }
    // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

} // namespace lox::vm
