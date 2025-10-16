#include "compile.hpp"

#include "lox/ast/parse.hpp"

#include <string_view>

namespace lox::vm {

    auto Compiler::compile(std::string_view source, Chunk *chunk) noexcept -> bool {
        auto parser = lox::ast::Parser(source);
        auto result = parser.parse();
        if (!result.has_value()) {
            spdlog::error("Parser error: {}", result.error());
            return false;
        }

        const auto &program = result.value();

        for (const auto &stmt : program) {
            fmt::println("Parsed statement: {}", stmt->to_string());
        }

        return true;
    }

} // namespace lox::vm
