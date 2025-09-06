add_library(lox-core STATIC
	"lox/syntax/location.hpp"
	"lox/syntax/token.hpp"
	"lox/syntax/lex.hpp"
	"lox/syntax/token.cpp"
	"lox/syntax/lex.cpp")
target_include_directories(lox-core PUBLIC ".")
target_link_libraries(lox-core PRIVATE spdlog::spdlog)