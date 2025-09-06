#ifndef LOX_SYNTAX_LEX_HPP
#define LOX_SYNTAX_LEX_HPP

#include "token.hpp"

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace lox::syntax {

  struct Cursor final {
    std::string_view src;
    uint8_t pos;
  };

  class Scanner final {
  public:
    explicit Scanner(std::string_view source) noexcept;

    Scanner(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner& operator=(Scanner&&) = delete;

    ~Scanner() = default;

    auto get_next_token() noexcept -> std::expected<Token, std::string>;

  private:
    void skip_whitespace() noexcept;
    void advance() noexcept;

    auto scan_identifier() noexcept -> std::expected<Token, std::string>;
    auto scan_number() noexcept -> std::expected<Token, std::string>;
    auto scan_string() noexcept -> std::expected<Token, std::string>;

    std::string_view source_;
    Cursor cursor_;
  };

}

#endif // LOX_SYNTAX_LEX_HPP  