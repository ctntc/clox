#include "parse.hpp"

#include "expr.hpp"
#include "stmt.hpp"

#include <algorithm>
#include <lox/syntax/token.hpp>

#include <spdlog/spdlog.h>

#include <cstdint>
#include <expected>
#include <format>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace lox::ast {

Parser::Parser(std::vector<syntax::Token> tokens) noexcept
    : tokens_{std::move(tokens)}, current_{0} {
  spdlog::debug("Parser: Initializing with {} tokens", tokens_.size());
  initialize_parse_rules();
  spdlog::debug("Parser: Parse rules initialized");
}

auto Parser::parse() noexcept
    -> std::expected<std::vector<StmtPtr>, std::string> {
  spdlog::info("Parser: Beginning parse of {} tokens", tokens_.size());
  std::vector<StmtPtr> statements;

  while (!is_at_end()) {
    spdlog::debug("Parser: Parsing declaration at token {}: {}", current_,
                  current_token().to_string());
    auto stmt_result = parse_declaration();
    if (!stmt_result) {
      spdlog::error("Parser: Declaration parse failed: {}",
                    stmt_result.error());
      synchronize();
      return std::unexpected(stmt_result.error());
    }
    statements.push_back(std::move(stmt_result.value()));
    spdlog::debug(
        "Parser: Successfully parsed statement, now have {} statements",
        statements.size());
  }

  spdlog::info("Parser: Successfully parsed {} statements", statements.size());
  return statements;
}

auto Parser::current_token() const noexcept -> const syntax::Token & {
  return tokens_[current_];
}

auto Parser::previous_token() const noexcept -> const syntax::Token & {
  return tokens_[current_ - 1];
}

auto Parser::peek_token() const noexcept -> const syntax::Token & {
  if (current_ + 1 >= tokens_.size()) {
    return tokens_.back();
  }
  return tokens_[current_ + 1];
}

auto Parser::is_at_end() const noexcept -> bool {
  return current_token().kind == syntax::TokenKind::end_of_file;
}

auto Parser::advance() noexcept -> const syntax::Token & {
  if (!is_at_end()) {
    current_++;
  }
  return previous_token();
}

auto Parser::check(const syntax::TokenKind kind) const noexcept -> bool {
  if (is_at_end())
    return false;
  return current_token().kind == kind;
}

auto Parser::match(
    const std::initializer_list<syntax::TokenKind> kinds) noexcept -> bool {
  return std::ranges::all_of(kinds, [this](const auto kind) {
    if (check(kind))
      return true;
    return false;
  });
}

auto Parser::consume(const syntax::TokenKind kind,
                     const std::string &message) noexcept
    -> std::expected<syntax::Token, std::string> {
  if (check(kind)) {
    return advance();
  }

  return std::unexpected(std::format("Parse error: {} at token '{}'", message,
                                     current_token().to_string()));
}

auto Parser::parse_declaration() noexcept
    -> std::expected<StmtPtr, std::string> {
  spdlog::trace("Parser: parse_declaration at token: {}",
                current_token().to_string());

  if (check(syntax::TokenKind::keyword)) {
    const auto &keyword = current_token();
    if (keyword.lexeme == "var") {
      spdlog::debug("Parser: Parsing variable declaration");
      advance();
      return parse_var_declaration();
    }
    if (keyword.lexeme == "fun") {
      spdlog::debug("Parser: Parsing function declaration");
      advance();
      return parse_function_declaration();
    }
  }

  spdlog::trace("Parser: Not a declaration, parsing as statement");
  return parse_statement();
}

auto Parser::parse_var_declaration() noexcept
    -> std::expected<StmtPtr, std::string> {
  spdlog::debug("Parser: parse_var_declaration");

  auto name_result =
      consume(syntax::TokenKind::identifier, "Expected variable name");
  if (!name_result) {
    spdlog::error("Parser: Variable declaration missing name: {}",
                  name_result.error());
    return std::unexpected(name_result.error());
  }

  spdlog::debug("Parser: Variable name: {}", name_result.value().lexeme);

  ExprPtr initializer = nullptr;
  if (check(syntax::TokenKind::simple_operator) &&
      current_token().lexeme == "=") {
    spdlog::debug("Parser: Variable has initializer");
    advance();
    auto init_result = parse_expression();
    if (!init_result) {
      spdlog::error("Parser: Variable initializer parse failed: {}",
                    init_result.error());
      return std::unexpected(init_result.error());
    }
    initializer = std::move(init_result.value());
  }

  auto semicolon_result = consume(syntax::TokenKind::punctuation,
                                  "Expected ';' after variable declaration");
  if (!semicolon_result) {
    spdlog::error("Parser: Variable declaration missing semicolon: {}",
                  semicolon_result.error());
    return std::unexpected(semicolon_result.error());
  }

  spdlog::debug("Parser: Successfully parsed variable declaration: {}",
                name_result.value().lexeme);
  return std::make_unique<VarStatement>(name_result.value(),
                                        std::move(initializer));
}

auto Parser::parse_function_declaration() noexcept
    -> std::expected<StmtPtr, std::string> {
  auto name_result =
      consume(syntax::TokenKind::identifier, "Expected function name");
  if (!name_result) {
    return std::unexpected(name_result.error());
  }

  auto lparen_result = consume(syntax::TokenKind::punctuation,
                               "Expected '(' after function name");
  if (!lparen_result || lparen_result.value().lexeme != "(") {
    return std::unexpected("Expected '(' after function name");
  }

  std::vector<syntax::Token> parameters;
  if (!check(syntax::TokenKind::punctuation) || current_token().lexeme != ")") {
    do {
      auto param_result =
          consume(syntax::TokenKind::identifier, "Expected parameter name");
      if (!param_result) {
        return std::unexpected(param_result.error());
      }
      parameters.push_back(param_result.value());

      if (!check(syntax::TokenKind::punctuation) ||
          current_token().lexeme != ",") {
        break;
      }
      advance();
    } while (true);
  }

  auto rparen_result =
      consume(syntax::TokenKind::punctuation, "Expected ')' after parameters");
  if (!rparen_result || rparen_result.value().lexeme != ")") {
    return std::unexpected("Expected ')' after parameters");
  }

  auto lbrace_result = consume(syntax::TokenKind::punctuation,
                               "Expected '{' before function body");
  if (!lbrace_result || lbrace_result.value().lexeme != "{") {
    return std::unexpected("Expected '{' before function body");
  }

  std::vector<StmtPtr> body;
  while (!check(syntax::TokenKind::punctuation) ||
         current_token().lexeme != "}") {
    if (is_at_end()) {
      return std::unexpected("Unterminated function body");
    }

    auto stmt_result = parse_declaration();
    if (!stmt_result) {
      return std::unexpected(stmt_result.error());
    }
    body.push_back(std::move(stmt_result.value()));
  }

  auto rbrace_result = consume(syntax::TokenKind::punctuation,
                               "Expected '}' after function body");
  if (!rbrace_result) {
    return std::unexpected(rbrace_result.error());
  }

  return std::make_unique<FunctionDeclarationStatement>(
      name_result.value(), std::move(parameters), std::move(body));
}

auto Parser::parse_statement() noexcept -> std::expected<StmtPtr, std::string> {
  spdlog::trace("Parser: parse_statement at token: {}",
                current_token().to_string());

  if (check(syntax::TokenKind::keyword)) {
    const auto &keyword = current_token();
    if (keyword.lexeme == "print") {
      spdlog::debug("Parser: Parsing print statement");
      advance();
      return parse_print_statement();
    }
    if (keyword.lexeme == "if") {
      spdlog::debug("Parser: Parsing if statement");
      advance();
      return parse_if_statement();
    }
    if (keyword.lexeme == "while") {
      spdlog::debug("Parser: Parsing while statement");
      advance();
      return parse_while_statement();
    }
    if (keyword.lexeme == "for") {
      spdlog::debug("Parser: Parsing for statement");
      advance();
      return parse_for_statement();
    }
    if (keyword.lexeme == "return") {
      spdlog::debug("Parser: Parsing return statement");
      advance();
      return parse_return_statement();
    }
  }

  if (check(syntax::TokenKind::punctuation) && current_token().lexeme == "{") {
    spdlog::debug("Parser: Parsing block statement");
    return parse_block_statement();
  }

  spdlog::trace("Parser: Parsing expression statement");
  return parse_expression_statement();
}

auto Parser::parse_print_statement() noexcept
    -> std::expected<StmtPtr, std::string> {
  auto expr_result = parse_expression();
  if (!expr_result) {
    return std::unexpected(expr_result.error());
  }

  auto semicolon_result = consume(syntax::TokenKind::punctuation,
                                  "Expected ';' after print statement");
  if (!semicolon_result) {
    return std::unexpected(semicolon_result.error());
  }

  return std::make_unique<PrintStatement>(std::move(expr_result.value()));
}

auto Parser::parse_block_statement() noexcept
    -> std::expected<StmtPtr, std::string> {
  auto lbrace_result = consume(syntax::TokenKind::punctuation, "Expected '{'");
  if (!lbrace_result || lbrace_result.value().lexeme != "{") {
    return std::unexpected("Expected '{'");
  }

  std::vector<StmtPtr> statements;
  while (!check(syntax::TokenKind::punctuation) ||
         current_token().lexeme != "}") {
    if (is_at_end()) {
      return std::unexpected("Unterminated block");
    }

    auto stmt_result = parse_declaration();
    if (!stmt_result) {
      return std::unexpected(stmt_result.error());
    }
    statements.push_back(std::move(stmt_result.value()));
  }

  auto rbrace_result =
      consume(syntax::TokenKind::punctuation, "Expected '}' after block");
  if (!rbrace_result) {
    return std::unexpected(rbrace_result.error());
  }

  return std::make_unique<BlockStatement>(std::move(statements));
}

auto Parser::parse_if_statement() noexcept
    -> std::expected<StmtPtr, std::string> {
  auto lparen_result =
      consume(syntax::TokenKind::punctuation, "Expected '(' after 'if'");
  if (!lparen_result || lparen_result.value().lexeme != "(") {
    return std::unexpected("Expected '(' after 'if'");
  }

  auto condition_result = parse_expression();
  if (!condition_result) {
    return std::unexpected(condition_result.error());
  }

  auto rparen_result = consume(syntax::TokenKind::punctuation,
                               "Expected ')' after if condition");
  if (!rparen_result || rparen_result.value().lexeme != ")") {
    return std::unexpected("Expected ')' after if condition");
  }

  auto then_result = parse_statement();
  if (!then_result) {
    return std::unexpected(then_result.error());
  }

  StmtPtr else_branch = nullptr;
  if (check(syntax::TokenKind::keyword) && current_token().lexeme == "else") {
    advance();
    auto else_result = parse_statement();
    if (!else_result) {
      return std::unexpected(else_result.error());
    }
    else_branch = std::move(else_result.value());
  }

  return std::make_unique<IfStatement>(std::move(condition_result.value()),
                                       std::move(then_result.value()),
                                       std::move(else_branch));
}

auto Parser::parse_while_statement() noexcept
    -> std::expected<StmtPtr, std::string> {
  auto lparen_result =
      consume(syntax::TokenKind::punctuation, "Expected '(' after 'while'");
  if (!lparen_result || lparen_result.value().lexeme != "(") {
    return std::unexpected("Expected '(' after 'while'");
  }

  auto condition_result = parse_expression();
  if (!condition_result) {
    return std::unexpected(condition_result.error());
  }

  auto rparen_result = consume(syntax::TokenKind::punctuation,
                               "Expected ')' after while condition");
  if (!rparen_result || rparen_result.value().lexeme != ")") {
    return std::unexpected("Expected ')' after while condition");
  }

  auto body_result = parse_statement();
  if (!body_result) {
    return std::unexpected(body_result.error());
  }

  return std::make_unique<WhileStatement>(std::move(condition_result.value()),
                                          std::move(body_result.value()));
}

auto Parser::parse_for_statement() noexcept
    -> std::expected<StmtPtr, std::string> {
  auto lparen_result =
      consume(syntax::TokenKind::punctuation, "Expected '(' after 'for'");
  if (!lparen_result || lparen_result.value().lexeme != "(") {
    return std::unexpected("Expected '(' after 'for'");
  }

  StmtPtr initializer = nullptr;
  if (check(syntax::TokenKind::punctuation) && current_token().lexeme == ";") {
    advance();
    // No initializer
  } else if (check(syntax::TokenKind::keyword) &&
             current_token().lexeme == "var") {
    advance();
    auto init_result = parse_var_declaration();
    if (!init_result) {
      return std::unexpected(init_result.error());
    }
    initializer = std::move(init_result.value());
  } else {
    auto init_result = parse_expression_statement();
    if (!init_result) {
      return std::unexpected(init_result.error());
    }
    initializer = std::move(init_result.value());
  }

  ExprPtr condition = nullptr;
  if (!check(syntax::TokenKind::punctuation) || current_token().lexeme != ";") {
    auto cond_result = parse_expression();
    if (!cond_result) {
      return std::unexpected(cond_result.error());
    }
    condition = std::move(cond_result.value());
  }

  auto semicolon_result = consume(syntax::TokenKind::punctuation,
                                  "Expected ';' after for loop condition");
  if (!semicolon_result || semicolon_result.value().lexeme != ";") {
    return std::unexpected("Expected ';' after for loop condition");
  }

  ExprPtr increment = nullptr;
  if (!check(syntax::TokenKind::punctuation) || current_token().lexeme != ")") {
    auto inc_result = parse_expression();
    if (!inc_result) {
      return std::unexpected(inc_result.error());
    }
    increment = std::move(inc_result.value());
  }

  auto rparen_result =
      consume(syntax::TokenKind::punctuation, "Expected ')' after for clauses");
  if (!rparen_result || rparen_result.value().lexeme != ")") {
    return std::unexpected("Expected ')' after for clauses");
  }

  auto body_result = parse_statement();
  if (!body_result) {
    return std::unexpected(body_result.error());
  }

  return std::make_unique<ForStatement>(
      std::move(initializer), std::move(condition), std::move(increment),
      std::move(body_result.value()));
}

auto Parser::parse_return_statement() noexcept
    -> std::expected<StmtPtr, std::string> {
  const auto keyword = previous_token();

  ExprPtr value = nullptr;
  if (!check(syntax::TokenKind::punctuation) || current_token().lexeme != ";") {
    auto val_result = parse_expression();
    if (!val_result) {
      return std::unexpected(val_result.error());
    }
    value = std::move(val_result.value());
  }

  auto semicolon_result = consume(syntax::TokenKind::punctuation,
                                  "Expected ';' after return value");
  if (!semicolon_result) {
    return std::unexpected(semicolon_result.error());
  }

  return std::make_unique<ReturnStatement>(keyword, std::move(value));
}

auto Parser::parse_expression_statement() noexcept
    -> std::expected<StmtPtr, std::string> {
  auto expr_result = parse_expression();
  if (!expr_result) {
    return std::unexpected(expr_result.error());
  }

  auto semicolon_result =
      consume(syntax::TokenKind::punctuation, "Expected ';' after expression");
  if (!semicolon_result) {
    return std::unexpected(semicolon_result.error());
  }

  return std::make_unique<ExpressionStatement>(std::move(expr_result.value()));
}

auto Parser::parse_expression() noexcept
    -> std::expected<ExprPtr, std::string> {
  spdlog::trace("Parser: parse_expression starting at token: {}",
                current_token().to_string());
  auto result = parse_expression_with_precedence(Precedence::assignment);
  if (result) {
    spdlog::trace("Parser: parse_expression succeeded");
  } else {
    spdlog::warn("Parser: parse_expression failed: {}", result.error());
  }
  return result;
}

auto Parser::parse_expression_with_precedence(Precedence precedence) noexcept
    -> std::expected<ExprPtr, std::string> {

  const auto &current = current_token();
  const auto &rule = get_rule(current.kind);

  spdlog::trace("Parser: parse_expression_with_precedence({}) at token: {}",
                static_cast<int>(precedence), current.to_string());

  if (!rule.prefix) {
    spdlog::error("Parser: No prefix rule for token: {}", current.to_string());
    return std::unexpected(std::format("Unexpected token '{}' in expression",
                                       current.to_string()));
  }

  advance();
  spdlog::trace("Parser: Calling prefix parser for token kind: {}",
                static_cast<int>(current.kind));
  auto left_result = rule.prefix();
  if (!left_result) {
    spdlog::error("Parser: Prefix parse failed: {}", left_result.error());
    return std::unexpected(left_result.error());
  }

  auto left = std::move(left_result.value());

  while (static_cast<uint8_t>(precedence) <=
         static_cast<uint8_t>(get_precedence(current_token().kind))) {
    const auto &infix_rule = get_rule(current_token().kind);
    if (!infix_rule.infix) {
      spdlog::trace(
          "Parser: No infix rule for token: {}, stopping precedence climb",
          current_token().to_string());
      break;
    }

    spdlog::trace("Parser: Continuing precedence climb with infix token: {}",
                  current_token().to_string());
    advance();
    auto right_result = infix_rule.infix(std::move(left));
    if (!right_result) {
      spdlog::error("Parser: Infix parse failed: {}", right_result.error());
      return std::unexpected(right_result.error());
    }
    left = std::move(right_result.value());
  }

  spdlog::trace("Parser: parse_expression_with_precedence completed");
  return left;
}

auto Parser::parse_grouping() noexcept -> std::expected<ExprPtr, std::string> {
  auto expr_result = parse_expression();
  if (!expr_result) {
    return std::unexpected(expr_result.error());
  }

  auto rparen_result =
      consume(syntax::TokenKind::punctuation, "Expected ')' after expression");
  if (!rparen_result || rparen_result.value().lexeme != ")") {
    return std::unexpected("Expected ')' after expression");
  }

  return std::make_unique<GroupingExpression>(std::move(expr_result.value()));
}

auto Parser::parse_unary() noexcept -> std::expected<ExprPtr, std::string> {
  const auto operator_token = previous_token();

  auto operand_result = parse_expression_with_precedence(Precedence::unary);
  if (!operand_result) {
    return std::unexpected(operand_result.error());
  }

  return std::make_unique<UnaryExpression>(operator_token,
                                           std::move(operand_result.value()));
}

auto Parser::parse_binary(ExprPtr left) noexcept
    -> std::expected<ExprPtr, std::string> {
  const auto operator_token = previous_token();
  spdlog::trace("Parser: parse_binary with operator: {}",
                operator_token.lexeme);

  // Determine precedence based on operator
  auto precedence = Precedence::term;
  const auto &lexeme = operator_token.lexeme;
  if (lexeme == "*" || lexeme == "/") {
    precedence = Precedence::factor;
  } else if (lexeme == "+" || lexeme == "-") {
    precedence = Precedence::term;
  }

  spdlog::trace("Parser: Binary operator '{}' has precedence {}", lexeme,
                static_cast<int>(precedence));

  auto right_result = parse_expression_with_precedence(
      static_cast<Precedence>(static_cast<uint8_t>(precedence) + 1));
  if (!right_result) {
    spdlog::error("Parser: Binary right operand parse failed: {}",
                  right_result.error());
    return std::unexpected(right_result.error());
  }

  spdlog::debug("Parser: Created binary expression with operator: {}", lexeme);
  return std::make_unique<BinaryExpression>(std::move(left), operator_token,
                                            std::move(right_result.value()));
}

auto Parser::parse_logical(ExprPtr left) noexcept
    -> std::expected<ExprPtr, std::string> {
  const auto operator_token = previous_token();

  // Determine precedence based on logical operator
  auto precedence = Precedence::logical_and;
  if (operator_token.lexeme == "or") {
    precedence = Precedence::logical_or;
  }

  auto right_result = parse_expression_with_precedence(
      static_cast<Precedence>(static_cast<uint8_t>(precedence) + 1));
  if (!right_result) {
    return std::unexpected(right_result.error());
  }

  return std::make_unique<LogicalExpression>(std::move(left), operator_token,
                                             std::move(right_result.value()));
}

auto Parser::parse_assignment(ExprPtr left) noexcept
    -> std::expected<ExprPtr, std::string> {
  const auto equals = previous_token();

  auto value_result = parse_expression_with_precedence(Precedence::assignment);
  if (!value_result) {
    return std::unexpected(value_result.error());
  }

  if (const auto var_expr = dynamic_cast<VariableExpression *>(left.get())) {
    const auto &name = var_expr->name;
    auto _ = left.release();
    return std::make_unique<AssignmentExpression>(
        name, std::move(value_result.value()));
  }

  return std::unexpected("Invalid assignment target");
}

auto Parser::parse_call(ExprPtr left) noexcept
    -> std::expected<ExprPtr, std::string> {
  return finish_call(std::move(left));
}

auto Parser::finish_call(ExprPtr callee) noexcept
    -> std::expected<ExprPtr, std::string> {
  std::vector<ExprPtr> arguments;

  if (!check(syntax::TokenKind::punctuation) || current_token().lexeme != ")") {
    do {
      auto arg_result = parse_expression();
      if (!arg_result) {
        return std::unexpected(arg_result.error());
      }
      arguments.push_back(std::move(arg_result.value()));

      if (!check(syntax::TokenKind::punctuation) ||
          current_token().lexeme != ",") {
        break;
      }
      advance();
    } while (true);
  }

  auto paren_result =
      consume(syntax::TokenKind::punctuation, "Expected ')' after arguments");
  if (!paren_result || paren_result.value().lexeme != ")") {
    return std::unexpected("Expected ')' after arguments");
  }

  return std::make_unique<CallExpression>(
      std::move(callee), paren_result.value(), std::move(arguments));
}

auto Parser::parse_literal() const noexcept
    -> std::expected<ExprPtr, std::string> {
  const auto &token = previous_token();
  spdlog::trace("Parser: parse_literal with token: {} ({})", token.lexeme,
                static_cast<int>(token.kind));
  return std::make_unique<LiteralExpression>(token);
}

auto Parser::parse_variable() const noexcept
    -> std::expected<ExprPtr, std::string> {
  const auto &token = previous_token();
  spdlog::trace("Parser: parse_variable with name: {}", token.lexeme);
  return std::make_unique<VariableExpression>(token);
}

auto Parser::get_rule(const syntax::TokenKind kind) const noexcept
    -> const ParseRule & {
  static const ParseRule empty_rule{
      .prefix = nullptr, .infix = nullptr, .precedence = Precedence::none};

  if (const auto it = rules_.find(kind); it != rules_.end()) {
    return it->second;
  }

  return empty_rule;
}

auto Parser::get_precedence(const syntax::TokenKind kind) const noexcept
    -> Precedence {
  if (kind == syntax::TokenKind::simple_operator) {
    const auto &token = current_token();
    if (token.lexeme == "*" || token.lexeme == "/") {
      return Precedence::factor;
    }
    if (token.lexeme == "+" || token.lexeme == "-") {
      return Precedence::term;
    }
    if (token.lexeme == "<" || token.lexeme == ">" || token.lexeme == "!") {
      return Precedence::comparison;
    }
    if (token.lexeme == "=") {
      return Precedence::assignment;
    }
  }
  return get_rule(kind).precedence;
}

auto Parser::synchronize() noexcept -> void {
  spdlog::warn("Parser: Synchronizing after error at token: {}",
               current_token().to_string());
  advance();

  while (!is_at_end()) {
    if (previous_token().lexeme == ";") {
      spdlog::debug("Parser: Synchronized at semicolon");
      return;
    }

    if (current_token().kind == syntax::TokenKind::keyword) {
      if (const auto &lexeme = current_token().lexeme;
          lexeme == "class" || lexeme == "fun" || lexeme == "var" ||
          lexeme == "for" || lexeme == "if" || lexeme == "while" ||
          lexeme == "print" || lexeme == "return") {
        spdlog::debug("Parser: Synchronized at keyword: {}", lexeme);
        return;
      }
    }

    advance();
  }

  spdlog::warn("Parser: Reached end of file while synchronizing");
}

void Parser::initialize_parse_rules() noexcept {
  spdlog::trace("Parser: Initializing parse rules");
  rules_[syntax::TokenKind::punctuation] = ParseRule{
      [this]() -> std::expected<ExprPtr, std::string> {
        if (const auto &token = previous_token(); token.lexeme == "(") {
          return parse_grouping();
        }
        return std::unexpected("Unexpected punctuation in expression");
      },
      nullptr, Precedence::none};

  // // TODO: Add specific rules for different operator precedences.
  // auto add_operator_rule = [this](const std::string &op, Precedence prec) {
  //   // This will be handled by the general simple_operator rule above.
  // };

  rules_[syntax::TokenKind::simple_operator] = ParseRule{
      .prefix = [this]() -> std::expected<ExprPtr, std::string> {
        if (const auto &token = previous_token();
            token.lexeme == "-" || token.lexeme == "!") {
          return parse_unary();
        }
        return std::unexpected("Unexpected operator in prefix position");
      },
      .infix = [this](ExprPtr left) -> std::expected<ExprPtr, std::string> {
        const auto &token = previous_token();
        if (token.lexeme == "=") {
          return parse_assignment(std::move(left));
        }
        if (token.lexeme == "+" || token.lexeme == "-") {
          return parse_binary(std::move(left));
        }
        if (token.lexeme == "*" || token.lexeme == "/") {
          return parse_binary(std::move(left));
        }
        if (token.lexeme == "<" || token.lexeme == ">" || token.lexeme == "!") {
          return parse_binary(std::move(left));
        }
        return std::unexpected("Unexpected operator in infix position");
      },
      .precedence = Precedence::term};

  rules_[syntax::TokenKind::compound_operator] = ParseRule{
      .prefix = nullptr,
      .infix = [this](ExprPtr left) -> std::expected<ExprPtr, std::string> {
        return parse_binary(std::move(left));
      },
      .precedence = Precedence::equality};

  rules_[syntax::TokenKind::identifier] =
      ParseRule{.prefix = [this]() -> std::expected<ExprPtr, std::string> {
                  return parse_variable();
                },
                .infix = nullptr,
                .precedence = Precedence::none};

  rules_[syntax::TokenKind::number_literal] =
      ParseRule{.prefix = [this]() -> std::expected<ExprPtr, std::string> {
                  return parse_literal();
                },
                .infix = nullptr,
                .precedence = Precedence::none};

  rules_[syntax::TokenKind::string_literal] =
      ParseRule{.prefix = [this]() -> std::expected<ExprPtr, std::string> {
                  return parse_literal();
                },
                .infix = nullptr,
                .precedence = Precedence::none};

  rules_[syntax::TokenKind::keyword] = ParseRule{
      .prefix = [this]() -> std::expected<ExprPtr, std::string> {
        if (const auto &token = previous_token(); token.lexeme == "true" ||
                                                  token.lexeme == "false" ||
                                                  token.lexeme == "nil") {
          return parse_literal();
        }
        return std::unexpected("Unexpected keyword in expression");
      },
      .infix = [this](ExprPtr left) -> std::expected<ExprPtr, std::string> {
        const auto &token = previous_token();
        if (token.lexeme == "and") {
          return parse_logical(std::move(left));
        }
        if (token.lexeme == "or") {
          return parse_logical(std::move(left));
        }
        return std::unexpected("Unexpected keyword in infix position");
      },
      .precedence = Precedence::logical_or};
}

} // namespace lox::ast
