#include <cstdlib>
#include <exception>
#include <format>
#include <lox/ast/expr.hpp>
#include <lox/ast/parse.hpp>
#include <lox/ast/stmt.hpp>
#include <lox/syntax/lex.hpp>
#include <lox/syntax/token.hpp>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

static auto scan_tokens(const std::string &source) -> std::vector<lox::syntax::Token> {
    auto scanner = lox::syntax::Scanner(source);
    std::vector<lox::syntax::Token> tokens;

    while (true) {
        auto token_result = scanner.get_next_token();
        if (!token_result) {
            throw std::runtime_error(std::format("Lexing error: {}", token_result.error()));
        }

        tokens.push_back(token_result.value());
        if (token_result.value().kind == lox::syntax::TokenKind::end_of_file) {
            break;
        }
    }

    return tokens;
}

static auto test_parse_literal_expression() -> bool {
    const auto source = "42;";
    auto tokens = scan_tokens(source);

    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *expr_stmt = dynamic_cast<const lox::ast::ExpressionStatement *>(statements[0].get());
    if (!expr_stmt) {
        std::print("Expected expression statement\n");
        return false;
    }

    const auto *literal = dynamic_cast<const lox::ast::LiteralExpression *>(expr_stmt->expression.get());
    if (!literal) {
        std::print("Expected literal expression\n");
        return false;
    }

    if (literal->value.lexeme != "42") {
        std::print("Expected literal value '42', got '{}'\n", literal->value.lexeme);
        return false;
    }

    return true;
}

static auto test_parse_binary_expression() -> bool {
    const auto source = "2 + 3 * 4;";
    auto tokens = scan_tokens(source);

    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *expr_stmt = dynamic_cast<const lox::ast::ExpressionStatement *>(statements[0].get());
    if (!expr_stmt) {
        std::print("Expected expression statement\n");
        return false;
    }

    const auto *binary = dynamic_cast<const lox::ast::BinaryExpression *>(expr_stmt->expression.get());
    if (!binary) {
        std::print("Expected binary expression\n");
        return false;
    }

    if (binary->operator_token.lexeme != "+") {
        std::print("Expected '+' operator, got '{}'\n", binary->operator_token.lexeme);
        return false;
    }

    return true;
}

static auto test_parse_unary_expression() -> bool {
    const auto source = "-42;";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *expr_stmt = dynamic_cast<const lox::ast::ExpressionStatement *>(statements[0].get());
    if (!expr_stmt) {
        std::print("Expected expression statement\n");
        return false;
    }

    const auto *unary = dynamic_cast<const lox::ast::UnaryExpression *>(expr_stmt->expression.get());
    if (!unary) {
        std::print("Expected unary expression\n");
        return false;
    }

    if (unary->operator_token.lexeme != "-") {
        std::print("Expected '-' operator, got '{}'\n", unary->operator_token.lexeme);
        return false;
    }

    return true;
}

static auto test_parse_grouping_expression() -> bool {
    const auto source = "(2 + 3) * 4;";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *expr_stmt = dynamic_cast<const lox::ast::ExpressionStatement *>(statements[0].get());
    if (!expr_stmt) {
        std::print("Expected expression statement\n");
        return false;
    }

    const auto *binary = dynamic_cast<const lox::ast::BinaryExpression *>(expr_stmt->expression.get());
    if (!binary) {
        std::print("Expected binary expression\n");
        return false;
    }

    const auto *grouping = dynamic_cast<const lox::ast::GroupingExpression *>(binary->left.get());
    if (!grouping) {
        std::print("Expected grouping expression on left side\n");
        return false;
    }

    return true;
}

static auto test_parse_variable_declaration() -> bool {
    const auto source = "var x = 42;";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *var_stmt = dynamic_cast<const lox::ast::VarStatement *>(statements[0].get());
    if (!var_stmt) {
        std::print("Expected variable statement\n");
        return false;
    }

    if (var_stmt->name.lexeme != "x") {
        std::print("Expected variable name 'x', got '{}'\n", var_stmt->name.lexeme);
        return false;
    }

    if (!var_stmt->initializer) {
        std::print("Expected initializer\n");
        return false;
    }

    return true;
}

static auto test_parse_assignment_expression() -> bool {
    const auto source = "x = 42;";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *expr_stmt = dynamic_cast<const lox::ast::ExpressionStatement *>(statements[0].get());
    if (!expr_stmt) {
        std::print("Expected expression statement\n");
        return false;
    }

    const auto *assignment = dynamic_cast<const lox::ast::AssignmentExpression *>(expr_stmt->expression.get());
    if (!assignment) {
        std::print("Expected assignment expression\n");
        return false;
    }

    if (assignment->name.lexeme != "x") {
        std::print("Expected assignment target 'x', got '{}'\n", assignment->name.lexeme);
        return false;
    }

    return true;
}

static auto test_parse_print_statement() -> bool {
    const auto source = "print \"Hello, World!\";";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *print_stmt = dynamic_cast<const lox::ast::PrintStatement *>(statements[0].get());
    if (!print_stmt) {
        std::print("Expected print statement\n");
        return false;
    }

    const auto *literal = dynamic_cast<const lox::ast::LiteralExpression *>(print_stmt->expression.get());
    if (!literal) {
        std::print("Expected literal expression in print statement\n");
        return false;
    }

    return true;
}

static auto test_parse_if_statement() -> bool {
    const auto source = R"(if (true) print "yes"; else print "no";)";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *if_stmt = dynamic_cast<const lox::ast::IfStatement *>(statements[0].get());
    if (!if_stmt) {
        std::print("Expected if statement\n");
        return false;
    }

    if (!if_stmt->condition) {
        std::print("Expected condition in if statement\n");
        return false;
    }

    if (!if_stmt->then_branch) {
        std::print("Expected then branch in if statement\n");
        return false;
    }

    if (!if_stmt->else_branch) {
        std::print("Expected else branch in if statement\n");
        return false;
    }

    return true;
}

static auto test_parse_while_statement() -> bool {
    const auto source = "while (x < 10) x = x + 1;";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *while_stmt = dynamic_cast<const lox::ast::WhileStatement *>(statements[0].get());
    if (!while_stmt) {
        std::print("Expected while statement\n");
        return false;
    }

    if (!while_stmt->condition) {
        std::print("Expected condition in while statement\n");
        return false;
    }

    if (!while_stmt->body) {
        std::print("Expected body in while statement\n");
        return false;
    }

    return true;
}

static auto test_parse_function_declaration() -> bool {
    const auto source = "fun add(a, b) { return a + b; }";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *fun_stmt = dynamic_cast<const lox::ast::FunctionDeclarationStatement *>(statements[0].get());
    if (!fun_stmt) {
        std::print("Expected function declaration statement\n");
        return false;
    }

    if (fun_stmt->name.lexeme != "add") {
        std::print("Expected function name 'add', got '{}'\n", fun_stmt->name.lexeme);
        return false;
    }

    if (fun_stmt->parameters.size() != 2) {
        std::print("Expected 2 parameters, got {}\n", fun_stmt->parameters.size());
        return false;
    }

    if (fun_stmt->body.size() != 1) {
        std::print("Expected 1 statement in function body, got {}\n", fun_stmt->body.size());
        return false;
    }

    return true;
}

static auto test_parse_block_statement() -> bool {
    const auto source = "{ var x = 1; var y = 2; }";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *block_stmt = dynamic_cast<const lox::ast::BlockStatement *>(statements[0].get());
    if (!block_stmt) {
        std::print("Expected block statement\n");
        return false;
    }

    if (block_stmt->statements.size() != 2) {
        std::print("Expected 2 statements in block, got {}\n", block_stmt->statements.size());
        return false;
    }

    return true;
}

static auto test_parse_logical_expression() -> bool {
    const auto source = "true and false or true;";
    auto tokens = scan_tokens(source);
    auto parser = lox::ast::Parser(std::move(tokens));

    auto result = parser.parse();
    if (!result) {
        std::print("Parse error: {}\n", result.error());
        return false;
    }

    const auto &statements = result.value();
    if (statements.size() != 1) {
        std::print("Expected 1 statement, got {}\n", statements.size());
        return false;
    }

    const auto *expr_stmt = dynamic_cast<const lox::ast::ExpressionStatement *>(statements[0].get());
    if (!expr_stmt) {
        std::print("Expected expression statement\n");
        return false;
    }

    const auto *logical = dynamic_cast<const lox::ast::LogicalExpression *>(expr_stmt->expression.get());
    if (!logical) {
        std::print("Expected logical expression\n");
        return false;
    }

    if (logical->operator_token.lexeme != "or") {
        std::print("Expected 'or' operator at top level, got '{}'\n", logical->operator_token.lexeme);
        return false;
    }

    return true;
}

auto main() noexcept -> int {
    const std::vector<std::pair<std::string, bool (*)()>> tests = {
        {"parse_literal_expression", test_parse_literal_expression},
        {"parse_binary_expression", test_parse_binary_expression},
        {"parse_unary_expression", test_parse_unary_expression},
        {"parse_grouping_expression", test_parse_grouping_expression},
        {"parse_variable_declaration", test_parse_variable_declaration},
        {"parse_assignment_expression", test_parse_assignment_expression},
        {"parse_print_statement", test_parse_print_statement},
        {"parse_if_statement", test_parse_if_statement},
        {"parse_while_statement", test_parse_while_statement},
        {"parse_function_declaration", test_parse_function_declaration},
        {"parse_block_statement", test_parse_block_statement},
        {"parse_logical_expression", test_parse_logical_expression}};

    int failed_tests = 0;
    for (const auto &[name, test_func] : tests) {
        std::print("Running test: {}... ", name);
        try {
            if (test_func()) {
                std::println("PASSED");
            } else {
                std::println("FAILED");
                failed_tests++;
            }
        } catch (const std::exception &e) {
            std::println("FAILED (exception: {})", e.what());
            failed_tests++;
        }
    }

    std::println("\nTest Summary: {}/{} tests passed", (tests.size() - failed_tests), tests.size());

    return failed_tests == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
