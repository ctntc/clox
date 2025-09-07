#ifndef LOX_AST_PARSE_HPP
#define LOX_AST_PARSE_HPP

#include "expr.hpp"
#include "lox/syntax/token.hpp"
#include "stmt.hpp"

#include <cstdint>
#include <expected>
#include <functional>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

namespace lox::ast {

  enum class Precedence : uint8_t {
    none = 0,
    assignment = 1,
    logical_or = 2,
    logical_and = 3,
    equality = 4,
    comparison = 5,
    term = 6,
    factor = 7,
    unary = 8,
    call = 9,
    primary = 10
  };

  class Parser final {
  public:
    explicit Parser(std::vector<syntax::Token> tokens) noexcept;

    Parser(const Parser&) = delete;
    Parser(Parser&&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser& operator=(Parser&&) = delete;

    ~Parser() = default;

    auto parse() noexcept -> std::expected<std::vector<StmtPtr>, std::string>;

  private:
    using PrefixParseFn = std::function<std::expected<ExprPtr, std::string>()>;
    using InfixParseFn =
      std::function<std::expected<ExprPtr, std::string>(ExprPtr)>;

    struct ParseRule {
      PrefixParseFn prefix;
      InfixParseFn infix;
      Precedence precedence;
    };

    [[nodiscard]] auto current_token() const noexcept -> const syntax::Token&;
    [[nodiscard]] auto previous_token() const noexcept -> const syntax::Token&;
    [[nodiscard]] auto peek_token() const noexcept -> const syntax::Token&;
    [[nodiscard]] auto is_at_end() const noexcept -> bool;
    auto advance() noexcept -> const syntax::Token&;
    [[nodiscard]] auto check(syntax::TokenKind kind) const noexcept -> bool;
    [[nodiscard]] auto match(std::initializer_list<syntax::TokenKind> kinds) noexcept -> bool;
    [[nodiscard]] auto consume(syntax::TokenKind kind, const std::string& message) noexcept
      -> std::expected<syntax::Token, std::string>;

    auto parse_expression() noexcept -> std::expected<ExprPtr, std::string>;
    auto parse_expression_with_precedence(Precedence precedence) noexcept
      -> std::expected<ExprPtr, std::string>;

    auto parse_statement() noexcept -> std::expected<StmtPtr, std::string>;
    auto parse_declaration() noexcept -> std::expected<StmtPtr, std::string>;
    auto parse_var_declaration() noexcept -> std::expected<StmtPtr, std::string>;
    auto parse_function_declaration() noexcept
      -> std::expected<StmtPtr, std::string>;
    auto parse_expression_statement() noexcept
      -> std::expected<StmtPtr, std::string>;
    auto parse_print_statement() noexcept -> std::expected<StmtPtr, std::string>;
    auto parse_block_statement() noexcept -> std::expected<StmtPtr, std::string>;
    auto parse_if_statement() noexcept -> std::expected<StmtPtr, std::string>;
    auto parse_while_statement() noexcept -> std::expected<StmtPtr, std::string>;
    auto parse_for_statement() noexcept -> std::expected<StmtPtr, std::string>;
    auto parse_return_statement() noexcept -> std::expected<StmtPtr, std::string>;

    auto parse_grouping() noexcept -> std::expected<ExprPtr, std::string>;
    auto parse_unary() noexcept -> std::expected<ExprPtr, std::string>;
    auto parse_binary(ExprPtr left) noexcept
      -> std::expected<ExprPtr, std::string>;
    auto parse_logical(ExprPtr left) noexcept
      -> std::expected<ExprPtr, std::string>;
    auto parse_assignment(ExprPtr left) noexcept
      -> std::expected<ExprPtr, std::string>;
    auto parse_call(ExprPtr left) noexcept -> std::expected<ExprPtr, std::string>;
    auto parse_literal() const noexcept -> std::expected<ExprPtr, std::string>;
    auto parse_variable() const noexcept -> std::expected<ExprPtr, std::string>;

    auto finish_call(ExprPtr callee) noexcept
      -> std::expected<ExprPtr, std::string>;

    [[nodiscard]] auto get_rule(syntax::TokenKind kind) const noexcept -> const ParseRule&;
    [[nodiscard]] auto get_precedence(syntax::TokenKind kind) const noexcept -> Precedence;

    auto synchronize() noexcept -> void;

    std::vector<syntax::Token> tokens_;
    size_t current_;
    std::unordered_map<syntax::TokenKind, ParseRule> rules_;

    void initialize_parse_rules() noexcept;
  };

} // namespace lox::ast

#endif // LOX_AST_PARSE_HPP
