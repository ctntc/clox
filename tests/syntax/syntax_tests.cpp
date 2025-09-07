#include <cstdlib>
#include <format>
#include <iostream>
#include <lox/syntax/lex.hpp>
#include <lox/syntax/token.hpp>
#include <stdexcept>
#include <vector>

static auto test_lex_simple_snippet() -> std::vector<lox::syntax::Token> {
  const auto snippet = "var meaningOfLife = 42;";
  auto scanner = lox::syntax::Scanner(snippet);

  std::vector<lox::syntax::Token> tokens{0};

  while (true) {
    if (const auto token = scanner.get_next_token(); token.has_value()) {
      tokens.push_back(token.value());
      if (token.value().kind == lox::syntax::TokenKind::end_of_file) {
        break;
      }
    } else {
      throw std::runtime_error(std::format("Lexing error: {}", token.error()));
    }
  }

  return tokens;
}

auto main() noexcept -> int {
  if (const auto tokens = test_lex_simple_snippet(); tokens.size() != 6) {
    std::cerr << "Test failed: Expected 6 tokens, got " << tokens.size()
              << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
