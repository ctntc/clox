add_library("loxc_core" STATIC
 "lox/syntax/location.hpp"
 "lox/syntax/token.hpp"
 "lox/syntax/lex.hpp"
 "lox/syntax/token.cpp"
 "lox/syntax/lex.cpp"

 "lox/ast/stmt.hpp"
 "lox/ast/expr.hpp"
 "lox/ast/parse.hpp"
 "lox/ast/parse.cpp"

 "lox/vm/common.hpp"
 "lox/vm/chunk.hpp"
 "lox/vm/chunk.cpp"
 "lox/vm/value.hpp"
)
target_include_directories("loxc_core" PUBLIC ".")
target_link_libraries("loxc_core" PRIVATE spdlog::spdlog)
