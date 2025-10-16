#include "lox/vm/common.hpp"
#include "lox/vm/vm.hpp"

#include <spdlog/cfg/env.h>

#include <format>
#include <fstream>
#include <iterator>
#include <print>
#include <stdexcept>
#include <string>

/**
 *
 * @param filename The path to the file to read.
 * @return A string containing the file's contents.
 */
static auto read_to_string(std::string_view filename) -> std::string {
    const std::string filename_str(filename);
    std::ifstream file{filename_str, std::ios::binary};
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Failed to open file {}", filename_str));
    }

    // Read the file into a string.
    const std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    return contents;
}

auto main(const int argc, const char *argv[]) noexcept -> int {
    spdlog::cfg::load_env_levels();

    lox::vm::VirtualMachine vm;

    lox::vm::Chunk chunk;

    auto constant = chunk.add_constant(1.2);
    chunk.write(static_cast<uint8_t>(lox::vm::OpCode::OP_CONSTANT), 123);
    chunk.write(static_cast<uint8_t>(constant), 123);

    constant = chunk.add_constant(3.4);
    chunk.write(static_cast<uint8_t>(lox::vm::OpCode::OP_CONSTANT), 123);
    chunk.write(static_cast<uint8_t>(constant), 123);

    chunk.write(static_cast<uint8_t>(lox::vm::OpCode::OP_ADD), 123);

    constant = chunk.add_constant(5.6);
    chunk.write(static_cast<uint8_t>(lox::vm::OpCode::OP_CONSTANT), 123);
    chunk.write(static_cast<uint8_t>(constant), 123);

    chunk.write(static_cast<uint8_t>(lox::vm::OpCode::OP_DIVIDE), 123);
    chunk.write(static_cast<uint8_t>(lox::vm::OpCode::OP_NEGATE), 123);

    chunk.write(static_cast<uint8_t>(lox::vm::OpCode::OP_RETURN), 123);

    std::println("{}", lox::vm::interpret_result_to_string(vm.interpret(chunk)));
}
