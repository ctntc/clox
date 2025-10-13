#ifndef LOX_SYNTAX_LOCATION_HPP
#define LOX_SYNTAX_LOCATION_HPP

#include <optional>
#include <string_view>

namespace lox::syntax {

    struct Span {
        size_t start, end;

        [[nodiscard]] constexpr auto length() const noexcept -> size_t { return end - start; }
    };

    struct Location {
        Location(const Location &) = default;
        Location(Location &&) = default;
        auto operator=(const Location &) -> Location & = default;
        auto operator=(Location &&) -> Location & = default;
        ~Location() = default;

        std::optional<std::string_view> source_file_name;
        int column{}, line{};
    };

} // namespace lox::syntax

#endif // LOX_SYNTAX_LOCATION_HPP
