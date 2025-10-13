#ifndef LOX_SYNTAX_LEX_HPP
#define LOX_SYNTAX_LEX_HPP

#include "token.hpp"

#include <expected>
#include <string>
#include <string_view>

namespace lox::syntax {

    struct Cursor final {
        std::string_view src;
        size_t pos;
    };

    class Scanner final {
    public:
        explicit Scanner(std::string_view source) noexcept;

        Scanner(const Scanner &) = delete;
        Scanner(Scanner &&) = delete;
        auto operator=(const Scanner &) -> Scanner & = delete;
        auto operator=(Scanner &&) -> Scanner & = delete;

        ~Scanner() = default;

        auto get_next_token() noexcept -> std::expected<Token, std::string>;

    private:
        [[nodiscard]] auto is_at_end() const noexcept -> bool;
        [[nodiscard]] auto current_char() const noexcept -> char;
        [[nodiscard]] auto peek_char(size_t offset = 1) const noexcept -> char;
        auto match_char(char expected) noexcept -> bool;

        void skip_whitespace() noexcept;
        void advance() noexcept;

        auto scan_identifier() noexcept -> std::expected<Token, std::string>;
        auto scan_number() noexcept -> std::expected<Token, std::string>;
        auto scan_string() noexcept -> std::expected<Token, std::string>;
        auto scan_operator() noexcept -> std::expected<Token, std::string>;

        std::string_view source_;
        Cursor cursor_;
    };

} // namespace lox::syntax

#endif // LOX_SYNTAX_LEX_HPP
