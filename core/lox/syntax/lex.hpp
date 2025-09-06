#ifndef LOX_SYNTAX_LEX_HPP
#define LOX_SYNTAX_LEX_HPP

#include <string_view>

namespace lox::syntax {

  class Scanner final {
  public:
    explicit Scanner(std::string_view source) noexcept;

    Scanner(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner& operator=(Scanner&&) = delete;

    ~Scanner() = default;

  private:
    std::string_view source_; // As Unicode.
  };

}

#endif // LOX_SYNTAX_LEX_HPP