#include "token.hpp"

#include "location.hpp"

#include <string_view>

namespace lox::syntax {

  auto Token::make(const TokenKind k, const std::string_view l, const Span s) noexcept
    -> Token {
    return Token{ .kind = k, .lexeme = l, .span = s };
  }

}
