#include "vm.hpp"

#include "lox/vm/common.hpp"
#include "lox/vm/value.hpp"

#include <spdlog/spdlog.h>

#include <print>

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

    VirtualMachine::VirtualMachine() : ip(0), m_stack(), m_chunks() { m_stack.reserve(STACK_MAX); }

    auto VirtualMachine::interpret(Chunk chunk) noexcept -> InterpretResult {
        m_chunks.push_back(std::move(chunk));
        ip = 0;
        return run();
    }

    auto VirtualMachine::run() noexcept -> InterpretResult {
        while (true) {
#ifndef NDEBUG
            std::print("          ");
            for (const auto &slot : m_stack) {
                std::print("[ {} ]", slot);
            }
            std::println();
            m_chunks.back().disassemble_instruction(ip);
#endif
            uint8_t instruction = 0;
            switch (instruction = read_byte()) {
            case static_cast<uint8_t>(OpCode::OP_CONSTANT): {
                Value constant = read_constant();
                m_stack.push_back(constant);
            } break;
            case static_cast<uint8_t>(OpCode::OP_ADD): {
                perform_binary_operation(std::plus<>{});
            } break;
            case static_cast<uint8_t>(OpCode::OP_SUBTRACT): {
                perform_binary_operation(std::minus<>{});
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_MULTIPLY): {
                perform_binary_operation(std::multiplies<>{});
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_DIVIDE): {
                perform_binary_operation(std::divides<>{});
            } break;
            case static_cast<uint8_t>(OpCode::OP_NEGATE): {
                if (m_stack.empty()) {
                    spdlog::error("Stack underflow");
                    return InterpretResult::runtime_error;
                }
                m_stack.back() = -m_stack.back();
            } break;
            case static_cast<uint8_t>(OpCode::OP_RETURN): {
                std::println("{}", m_stack.back());
                return InterpretResult::ok;
            }
            }
        }
    }

    [[nodiscard]] auto VirtualMachine::read_byte() noexcept -> uint8_t { return m_chunks.back().code[ip++]; }
    [[nodiscard]] auto VirtualMachine::read_constant() noexcept -> Value {
        return m_chunks.back().constants[read_byte()];
    }

} // namespace lox::vm
