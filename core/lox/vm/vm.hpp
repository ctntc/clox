#ifndef LOX_VM_VM_HPP
#define LOX_VM_VM_HPP

#include "lox/vm/chunk.hpp"

#include <spdlog/spdlog.h>

#include <vector>

namespace lox::vm {

    constexpr size_t STACK_MAX = 256;

    enum class InterpretResult : uint8_t { ok, compile_error, runtime_error };

    [[nodiscard]] auto interpret_result_to_string(InterpretResult result) noexcept -> std::string_view;

    class VirtualMachine final {

    public:
        VirtualMachine();
        ~VirtualMachine() = default;

        VirtualMachine(const VirtualMachine &) = delete;
        VirtualMachine(VirtualMachine &&) = delete;
        auto operator=(const VirtualMachine &) -> VirtualMachine & = delete;
        auto operator=(VirtualMachine &&) -> VirtualMachine & = delete;

        [[nodiscard]] auto interpret(std::string_view source) noexcept -> InterpretResult;

    private:
        [[nodiscard]] auto run() noexcept -> InterpretResult;

        [[nodiscard]] auto read_byte() noexcept -> uint8_t;
        [[nodiscard]] auto read_constant() noexcept -> Value;

        template <typename BinOp> auto perform_binary_operation(BinOp op) noexcept -> void {
            if (m_stack.size() < 2) {
                spdlog::error("Stack underflow");
                return;
            }
            const Value b = m_stack.back();
            m_stack.pop_back();
            const Value a = m_stack.back();
            m_stack.pop_back();
            const Value result = op(a, b);
            m_stack.push_back(result);
        }

    private:
        size_t ip;
        std::vector<Chunk> m_chunks;
        std::vector<Value> m_stack;
    };

} // namespace lox::vm

#endif
