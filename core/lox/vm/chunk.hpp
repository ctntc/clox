#ifndef LOX_VM_CHUNK_HPP
#define LOX_VM_CHUNK_HPP

#include "value.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace lox::vm {

    struct Chunk {
        size_t count;
        size_t capacity;
        std::vector<std::uint8_t> code;
        std::vector<Value> constants;

        explicit Chunk();
        Chunk(const Chunk &) = default;
        Chunk(Chunk &&) = delete;
        auto operator=(const Chunk &) -> Chunk & = default;
        auto operator=(Chunk &&) -> Chunk & = delete;
        ~Chunk() = default;

        auto write(uint8_t byte) noexcept -> void;
        auto add_constant(Value value) noexcept -> size_t;

        auto disassemble(std::string_view name) noexcept -> void;
        auto disassemble_instruction(size_t offset) noexcept -> size_t;
    };

} // namespace lox::vm

#endif
