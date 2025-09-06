#ifndef LOX_SYNTAX_LOCATION_HPP
#define LOX_SYNTAX_LOCATION_HPP

#include <cstdint>
#include <optional>
#include <string_view>

namespace lox::syntax {

  struct Span {
    uint8_t start, end;

    [[nodiscard]] constexpr auto length() const noexcept -> uint8_t {
      return end - start;
    }
  };

  struct Location {
    std::optional<std::string_view> source_file_name;
    int column, line;
  };

}

#endif // LOX_SYNTAX_LOCATION_HPP