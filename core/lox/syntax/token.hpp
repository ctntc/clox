#ifndef LOX_SYNTAX_TOKEN_HPP
#define LOX_SYNTAX_TOKEN_HPP

#include "lox/syntax/location.hpp"

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
        simple_operator,
        compound_operator,
        keyword,

        end_of_file,
    };

    inline const std::unordered_map<std::string_view, TokenKind> keywords{
        {"and", TokenKind::keyword},   {"class", TokenKind::keyword},  {"else", TokenKind::keyword},
        {"false", TokenKind::keyword}, {"for", TokenKind::keyword},    {"fun", TokenKind::keyword},
        {"if", TokenKind::keyword},    {"nil", TokenKind::keyword},    {"or", TokenKind::keyword},
        {"print", TokenKind::keyword}, {"return", TokenKind::keyword}, {"super", TokenKind::keyword},
        {"this", TokenKind::keyword},  {"true", TokenKind::keyword},   {"var", TokenKind::keyword},
        {"while", TokenKind::keyword},
    };

    static constexpr auto token_kind_to_string(const TokenKind &tk) noexcept -> std::string_view {
        switch (tk) {
        case TokenKind::string_literal:
            return "string-literal";
        case TokenKind::number_literal:
            return "number-literal";
        case TokenKind::identifier:
            return "identifier";
        case TokenKind::punctuation:
            return "punctuation";
        case TokenKind::simple_operator:
            return "simple-operator";
        case TokenKind::compound_operator:
            return "compound-operator";
        case TokenKind::keyword:
            return "keyword";
        case TokenKind::end_of_file:
            return "EOF";
        }

        std::unreachable();
    }

    struct Token {
        TokenKind kind = TokenKind::end_of_file;
        std::string lexeme;
        Span span{.start = 0, .end = 0};

        static auto make(TokenKind k, std::string_view l, Span s) noexcept -> Token;
        static auto make_eof(Span s = {.start = 1, .end = 1}) noexcept -> Token;

        [[nodiscard]] auto to_string() const noexcept -> std::string;
    };

} // namespace lox::syntax

#endif // LOX_SYNTAX_TOKEN_HPP
