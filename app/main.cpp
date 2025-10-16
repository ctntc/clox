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

    if (argc != 2) {
        std::println("Usage: clox <script>");
        return 64;
    }

    lox::vm::VirtualMachine vm;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto source = read_to_string(argv[1]);

    std::println("{}", lox::vm::interpret_result_to_string(vm.interpret(source)));
}
