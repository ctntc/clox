add_executable(syntax_test "syntax/syntax_tests.cpp")
target_link_libraries(syntax_test PRIVATE lox-core spdlog::spdlog)
add_test(NAME syntax_test COMMAND syntax_test)

add_executable(parser_test "ast/parser_tests.cpp")
target_link_libraries(parser_test PRIVATE lox-core spdlog::spdlog)
add_test(NAME parser_test COMMAND parser_test)
