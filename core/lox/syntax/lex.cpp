#include "lex.hpp"

#include "location.hpp"
#include "token.hpp"

#include <cctype>
#include <cstdint>
#include <expected>
#include <format>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

namespace lox::syntax {

    Scanner::Scanner(const std::string_view source) noexcept : source_{source}, cursor_{.src = source, .pos = 0} {
        spdlog::debug("Scanner: Initialized with source of length {}", source_.length());
    }

    void Scanner::skip_whitespace() noexcept {
        while (!is_at_end() && std::isspace(current_char())) {
            advance();
        }
    }

    void Scanner::advance() noexcept {
        if (!is_at_end()) {
            cursor_.pos++;
        }
    }

    auto Scanner::is_at_end() const noexcept -> bool { return cursor_.pos >= source_.length(); }

    auto Scanner::current_char() const noexcept -> char { return is_at_end() ? '\0' : source_[cursor_.pos]; }

    auto Scanner::peek_char(const size_t offset) const noexcept -> char {
        const size_t pos = static_cast<size_t>(cursor_.pos) + static_cast<size_t>(offset);
        return pos >= source_.length() ? '\0' : source_[pos];
    }

    auto Scanner::match_char(const char expected) noexcept -> bool {
        if (is_at_end() || current_char() != expected) {
            return false;
        }
        advance();
        return true;
    }

    auto Scanner::scan_identifier() noexcept -> std::expected<Token, std::string> {
        const auto start = cursor_.pos;

        while (!is_at_end() && (std::isalnum(current_char()) || current_char() == '_')) {
            advance();
        }

        const auto lexeme = source_.substr(start, cursor_.pos - start);
        const auto span = Span{.start = start, .end = cursor_.pos};

        if (const auto it = keywords.find(lexeme); it != keywords.end()) {
            spdlog::trace("Scanner: Scanned keyword '{}' at span [{}, {})", lexeme, span.start, span.end);
            return Token::make(it->second, lexeme, span);
        }

        spdlog::trace("Scanner: Scanned identifier '{}' at span [{}, {})", lexeme, span.start, span.end);
        return Token::make(TokenKind::identifier, lexeme, span);
    }

    auto Scanner::scan_number() noexcept -> std::expected<Token, std::string> {
        const auto start = cursor_.pos;

        while (!is_at_end() && std::isdigit(current_char())) {
            advance();
        }

        if (current_char() == '.' && std::isdigit(peek_char(1))) {
            advance();
            while (!is_at_end() && std::isdigit(current_char())) {
                advance();
            }
        }

        const auto lexeme = source_.substr(start, cursor_.pos - start);
        const auto span = Span{.start = start, .end = cursor_.pos};

        spdlog::trace("Scanner: Scanned number '{}' at span [{}, {})", lexeme, span.start, span.end);
        return Token::make(TokenKind::number_literal, lexeme, span);
    }

    auto Scanner::scan_string() noexcept -> std::expected<Token, std::string> {
        const auto start = cursor_.pos - 1;
        while (!is_at_end() && current_char() != '"') {
            advance();
        }

        if (is_at_end()) {
            spdlog::error("Scanner: Unterminated string literal starting at position {}", start);
            return std::unexpected("Unterminated string literal");
        }

        const auto content_start = start + 1;
        const auto content_length = cursor_.pos - content_start;
        const auto lexeme = source_.substr(content_start, content_length);

        advance();
        const auto span = Span{.start = static_cast<uint8_t>(start), .end = cursor_.pos};

        spdlog::trace("Scanner: Scanned string '{}' at span [{}, {})", lexeme, span.start, span.end);
        return Token::make(TokenKind::string_literal, lexeme, span);
    }

    auto Scanner::scan_operator() noexcept -> std::expected<Token, std::string> {
        const auto start = cursor_.pos;
        const char first_char = current_char();
        advance();

        auto kind = TokenKind::simple_operator;
        uint8_t length = 1;

        switch (first_char) {
        case '=':
            if (match_char('=')) {
                kind = TokenKind::compound_operator;
                length = 2;
            }
            break;
        case '!':
        case '<':
        case '>':
            if (match_char('=')) {
                kind = TokenKind::compound_operator;
                length = 2;
            }
            break;
        case '+':
        case '-':
        case '*':
        case '/':
            break;
        default:
            spdlog::error("Scanner: Unknown operator '{}' at position {}", first_char, start);
            return std::unexpected(std::format("Unknown operator '{}' at position {}", first_char, start));
        }

        const auto lexeme = source_.substr(start, length);
        const auto span = Span{.start = start, .end = cursor_.pos};

        const auto kind_str = (kind == TokenKind::compound_operator) ? "compound operator" : "simple operator";
        spdlog::trace("Scanner: Scanned {} '{}' at span [{}, {})", kind_str, lexeme, span.start, span.end);

        return Token::make(kind, lexeme, span);
    }

    auto Scanner::get_next_token() noexcept -> std::expected<Token, std::string> {
        skip_whitespace();

        if (is_at_end()) {
            spdlog::trace("Scanner: Reached end of file, returning EOF token");
            return Token::make_eof({.start = cursor_.pos, .end = cursor_.pos});
        }

        const char current = current_char();

        if (std::isalpha(current) || current == '_') {
            return scan_identifier();
        }

        if (std::isdigit(current)) {
            return scan_number();
        }

        if (current == '"') {
            advance();
            return scan_string();
        }

        if (current == '(' || current == ')' || current == '{' || current == '}' || current == ';' || current == ',') {
            const auto start = cursor_.pos;
            advance();
            const auto lexeme = source_.substr(start, 1);
            const auto span = Span{.start = start, .end = cursor_.pos};

            spdlog::trace("Scanner: Scanned punctuation '{}' at span [{}, {})", lexeme, span.start, span.end);
            return Token::make(TokenKind::punctuation, lexeme, span);
        }

        if (current == '+' || current == '-' || current == '*' || current == '/' || current == '=' || current == '!' ||
            current == '<' || current == '>') {
            return scan_operator();
        }

        const auto start = cursor_.pos;
        advance();
        spdlog::error("Scanner: Unexpected character '{}' at position {}", current, start);
        return std::unexpected(std::format("Unexpected character '{}' at position {}", current, start));
    }

} // namespace lox::syntax
