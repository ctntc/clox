#ifndef LOX_SYNTAX_LOCATION_HPP
#define LOX_SYNTAX_LOCATION_HPP

#include <optional>
#include <string_view>

namespace lox::syntax {

	struct Span {
		size_t start, end;

		[[nodiscard]] constexpr auto length() const noexcept -> size_t {
			return end - start;
		}
	};

	struct Location {
		std::optional<std::string_view> source_file_name;
		int column, line;
	};

} // namespace lox::syntax

#endif // LOX_SYNTAX_LOCATION_HPP
