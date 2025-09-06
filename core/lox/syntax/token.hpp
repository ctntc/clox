#ifndef LOX_SYNTAX_TOKEN_HPP
#define LOX_SYNTAX_TOKEN_HPP

#include "location.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace lox::syntax {

  enum class TokenKind : uint8_t {
    string_literal,
    number_literal,
    identifier,

    punctuation,
    keyword,

    end_of_file,
  };

  inline const std::unordered_map<std::string, TokenKind> keywords{
      {"and", TokenKind::keyword},    {"class", TokenKind::keyword},
      {"else", TokenKind::keyword},   {"false", TokenKind::keyword},
      {"for", TokenKind::keyword},    {"fun", TokenKind::keyword},
      {"if", TokenKind::keyword},     {"nil", TokenKind::keyword},
      {"or", TokenKind::keyword},     {"print", TokenKind::keyword},
      {"return", TokenKind::keyword}, {"super", TokenKind::keyword},
      {"this", TokenKind::keyword},   {"true", TokenKind::keyword},
      {"var", TokenKind::keyword},    {"while", TokenKind::keyword},
  };

  static constexpr auto token_kind_to_string(const TokenKind& tk) noexcept
    -> std::string_view {
    switch (tk) {
    case TokenKind::string_literal:
      return "string-literal";
    case TokenKind::number_literal:
      return "number-literal";
    case TokenKind::identifier:
      return "identifier";
    case TokenKind::punctuation:
      return "punctuation";
    case TokenKind::keyword:
      return "keyword";
    case TokenKind::end_of_file:
      return "EOF";
    }

    std::unreachable();
  }

  struct Token {
    TokenKind kind;
    std::string_view lexeme;
    Span span;

    static auto make(TokenKind k, std::string_view l, Span s) noexcept
      -> Token;
  };

}

#endif // LOX_SYNTAX_TOKEN_HPP