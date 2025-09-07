#include <lox/ast/parse.hpp>
#include <lox/syntax/lex.hpp>
#include <lox/syntax/token.hpp>

#include <spdlog/cfg/env.h>

#include <cstdlib>
#include <exception>
#include <format>
#include <fstream>
#include <iterator>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

/**
 *
 * @param filename The path to the file to read.
 * @return A string containing the file's contents.
 */
static auto read_to_string(const char filename[]) -> std::string {
    std::ifstream file{filename, std::ios::binary};
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Failed to open file {}", filename));
    }

    // Read the file into a string.
    const std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    return contents;
}

auto main(const int argc, const char *argv[]) noexcept -> int {
    spdlog::cfg::load_env_levels();

    try {
        const auto contents = read_to_string(R"(D:\Projects\clox\test.lox)");

        auto scanner = lox::syntax::Scanner(contents);
        std::vector<lox::syntax::Token> tokens;
        while (true) {

            if (auto token = scanner.get_next_token(); token.has_value()) {
                tokens.push_back(token.value());
                if (token.value().kind == lox::syntax::TokenKind::end_of_file) {
                    break;
                }
            } else {
                throw std::runtime_error(std::format("Lexing error: {}", token.error()));
            }
        }

        auto parser = lox::ast::Parser(std::move(tokens));
        auto result = parser.parse();
        if (!result) {
            throw std::runtime_error(std::format("Parse error: {}", result.error()));
        }

        const auto &statements = result.value();
        for (const auto &stmt : statements) {
            std::println("{}", stmt->to_string());
        }

    } catch (const std::exception &e) {
        std::println("Error: {}", e.what());
        return EXIT_FAILURE;
    }
}
