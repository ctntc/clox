#include "lex.hpp"

#include <string_view>

#include "location.hpp"
#include "token.hpp"
#include <cctype>
#include <cstdint>
#include <expected>
#include <format>
#include <spdlog/spdlog.h>
#include <string>

namespace lox::syntax {

  Scanner::Scanner(const std::string_view source) noexcept
    : source_{ source }, cursor_{ .src = source, .pos = 0 } {
    spdlog::debug("Scanner initialized with source of length {}", source_.length());
  }

  void Scanner::skip_whitespace() noexcept {
    while (cursor_.pos < source_.length() &&
      std::isspace(source_[cursor_.pos])) {
      cursor_.pos++;
    }
  }

  void Scanner::advance() noexcept {
    if (cursor_.pos < source_.length()) {
      cursor_.pos++;
    }
  }

  auto Scanner::scan_identifier() noexcept -> std::expected<Token, std::string> {
    const auto start = cursor_.pos;

    while (cursor_.pos < source_.length() &&
      (std::isalnum(source_[cursor_.pos]) || source_[cursor_.pos] == '_')) {
      cursor_.pos++;
    }

    const auto lexeme = source_.substr(start, cursor_.pos - start);
    const auto span = Span{ .start = start, .end = cursor_.pos };

    if (keywords.contains(lexeme.data())) {
      spdlog::debug("Scanned keyword: '{}' at span [{}, {})", lexeme, span.start, span.end);

      return Token::make(keywords.at(lexeme.data()), lexeme, span);
    }

    spdlog::debug("Scanned identifier: '{}' at span [{}, {})", lexeme, span.start, span.end);

    return Token::make(TokenKind::identifier, lexeme, span);
  }


  auto Scanner::scan_number() noexcept -> std::expected<Token, std::string> {
    const auto start = cursor_.pos;

    while (cursor_.pos < source_.length() && std::isdigit(source_[cursor_.pos])) {
      cursor_.pos++;
    }

    const auto lexeme = source_.substr(start, cursor_.pos - start);
    const auto span = Span{ .start = start, .end = cursor_.pos };

    spdlog::debug("Scanned number: '{}' at span [{}, {})", lexeme, span.start, span.end);

    return Token::make(TokenKind::number_literal, lexeme, span);
  }

  auto Scanner::scan_string() noexcept -> std::expected<Token, std::string> {
    const auto start = cursor_.pos - 1; // Include opening quote in span

    while (cursor_.pos < source_.length() && source_[cursor_.pos] != '"') {
      cursor_.pos++;
    }

    if (cursor_.pos >= source_.length()) {
      return std::unexpected("Unterminated string literal");
    }

    // Get the string content (without quotes)
    const auto content_start = start + 1;
    const auto content_length = cursor_.pos - content_start;
    const auto lexeme = source_.substr(content_start, content_length);

    advance(); // Skip closing quote
    const auto span = Span{ .start = static_cast<uint8_t>(start), .end = cursor_.pos };

    spdlog::debug("Scanned string: '{}' at span [{}, {})", lexeme, span.start, span.end);

    return Token::make(TokenKind::string_literal, lexeme, span);
  }

  auto Scanner::get_next_token() noexcept -> std::expected<Token, std::string> {
    skip_whitespace();

    if (cursor_.pos >= source_.length()) {
      return Token::make_eof({ .start = cursor_.pos, .end = cursor_.pos });
    }

    const char current = source_[cursor_.pos];

    // Handle identifiers and keywords
    if (std::isalpha(current) || current == '_') {
      return scan_identifier();
    }

    // Handle numbers
    if (std::isdigit(current)) {
      return scan_number();
    }

    // Handle string literals
    if (current == '"') {
      advance(); // Skip opening quote
      return scan_string();
    }

    // Handle punctuation
    if (current == ')' || current == '(' || current == '{' || current == '}' || current == ';') {
      const auto start = cursor_.pos;
      advance();
      const auto lexeme = source_.substr(start, 1);
      const auto span = Span{ .start = start, .end = cursor_.pos };
      spdlog::debug("Scanned punctuation: '{}' at span [{}, {})", lexeme, span.start, span.end);
      return Token::make(TokenKind::punctuation, lexeme, span);
    }

    // Handle simple operators
    if (current == '+' || current == '-' || current == '*' || current == '/' || current == '=') {
      const auto start = cursor_.pos;
      advance();
      const auto lexeme = source_.substr(start, 1);
      const auto span = Span{ .start = start, .end = cursor_.pos };
      spdlog::debug("Scanned simple operator: '{}' at span [{}, {})", lexeme, span.start, span.end);
      return Token::make(TokenKind::simple_operator, lexeme, span);
    }

    // TODO: Handle compound operators like '==', '!=', '<=', '>='

    // Unknown character - advance and return error
    const auto start = cursor_.pos;
    advance();
    return std::unexpected(std::format("Unexpected character '{}' at position {}", current, start));
  }


} // namespace lox::syntax