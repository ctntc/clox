#ifndef LOX_AST_EXPR_HPP
#define LOX_AST_EXPR_HPP

#include "lox/syntax/token.hpp"

#include <format>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

namespace lox::ast {

struct Expression {
  virtual ~Expression() = default;
  virtual auto to_string() const noexcept -> std::string = 0;
};

using ExprPtr = std::unique_ptr<Expression>;

struct BinaryExpression : public Expression {
  ExprPtr left;
  ExprPtr right;
  syntax::Token operator_token;

  BinaryExpression(ExprPtr l, syntax::Token op, ExprPtr r) noexcept
      : left(std::move(l)), right(std::move(r)), operator_token(op) {
    spdlog::trace("AST: Created BinaryExpression with operator '{}'",
                  op.lexeme);
  }

  auto to_string() const noexcept -> std::string override {
    return std::format("binary-expr{{ left: {}, right: {}, op: {} }}",
                       left->to_string(), right->to_string(),
                       operator_token.to_string());
  }
};

struct UnaryExpression : public Expression {
  syntax::Token operator_token;
  ExprPtr operand;

  UnaryExpression(syntax::Token op, ExprPtr expr) noexcept
      : operator_token(op), operand(std::move(expr)) {
    spdlog::trace("AST: Created UnaryExpression with operator '{}'", op.lexeme);
  }

  auto to_string() const noexcept -> std::string override {
    return std::format("unary-expr{{ op: {}, operand: {} }}",
                       operator_token.to_string(), operand->to_string());
  }
};

struct GroupingExpression : public Expression {
  ExprPtr expression;

  explicit GroupingExpression(ExprPtr expr) noexcept
      : expression(std::move(expr)) {
    spdlog::trace("AST: Created GroupingExpression");
  }

  auto to_string() const noexcept -> std::string override {
    return std::format("grouping-expr{{ expr: {} }}", expression->to_string());
  }
};

struct LiteralExpression : public Expression {
  syntax::Token value;

  explicit LiteralExpression(syntax::Token val) noexcept : value(val) {
    spdlog::trace("AST: Created LiteralExpression with value '{}'", val.lexeme);
  }

  auto to_string() const noexcept -> std::string override {
    return std::format("literal-expr{{ value: {} }}", value.to_string());
  }
};

struct VariableExpression : public Expression {
  syntax::Token name;

  explicit VariableExpression(syntax::Token n) noexcept : name(n) {
    spdlog::trace("AST: Created VariableExpression with name '{}'", n.lexeme);
  }

  auto to_string() const noexcept -> std::string override {
    return std::format("variable-expr{{ name: {} }}", name.to_string());
  }
};

struct AssignmentExpression : public Expression {
  syntax::Token name;
  ExprPtr value;

  AssignmentExpression(syntax::Token n, ExprPtr val) noexcept
      : name(n), value(std::move(val)) {
    spdlog::trace("AST: Created AssignmentExpression to variable '{}'",
                  n.lexeme);
  }

  auto to_string() const noexcept -> std::string override {
    return std::format("assignment-expr{{ name: {}, value: {} }}",
                       name.to_string(), value->to_string());
  }
};

struct LogicalExpression : public Expression {
  ExprPtr left;
  syntax::Token operator_token;
  ExprPtr right;

  LogicalExpression(ExprPtr l, syntax::Token op, ExprPtr r) noexcept
      : left(std::move(l)), operator_token(op), right(std::move(r)) {
    spdlog::trace("AST: Created LogicalExpression with operator '{}'",
                  op.lexeme);
  }

  auto to_string() const noexcept -> std::string override {
    return std::format("logical-expr{{ left: {}, op: {}, right: {} }}",
                       left->to_string(), operator_token.to_string(),
                       right->to_string());
  }
};

struct CallExpression : public Expression {
  ExprPtr callee;
  syntax::Token paren;
  std::vector<ExprPtr> arguments;

  CallExpression(ExprPtr c, syntax::Token p, std::vector<ExprPtr> args) noexcept
      : callee(std::move(c)), paren(p), arguments(std::move(args)) {
    spdlog::trace("AST: Created CallExpression with {} arguments",
                  arguments.size());
  }

  auto to_string() const noexcept -> std::string override {
    std::string args_str;
    for (const auto &arg : arguments) {
      if (!args_str.empty())
        args_str += ", ";
      args_str += arg->to_string();
    }
    return std::format("call-expr{{ callee: {}, args: [{}] }}",
                       callee->to_string(), args_str);
  }
};

} // namespace lox::ast

#endif
