#include "lex.hpp"


#include <string_view>

namespace lox::syntax {

  Scanner::Scanner(const std::string_view source) noexcept
    : source_{ source } {
  }

}