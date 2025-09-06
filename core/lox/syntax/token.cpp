#include "token.hpp"

#include "location.hpp"

#include <format>
#include <string>
#include <string_view>

namespace lox::syntax {

  auto Token::make(const TokenKind k, const std::string_view l, const Span s) noexcept
    -> Token {
    return Token{ .kind = k, .lexeme = l, .span = s };
  }

  auto Token::make_eof(const Span s) noexcept -> Token {
    return Token{ .kind = TokenKind::end_of_file, .lexeme = "EOF", .span = s };
  }
  auto Token::to_string() noexcept -> std::string {
    return std::format("Token{{ kind: {}, lexeme: {}, span: [{}..{}) }}",
      token_kind_to_string(kind),
      lexeme,
      span.start,
      span.end);
  }

} // namespace lox::syntax
