#ifndef LOX_AST_STMT_HPP
#define LOX_AST_STMT_HPP

#include "expr.hpp"
#include "lox/syntax/token.hpp"

#include <format>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>
#include <vector>

namespace lox::ast {

    struct Statement {
        Statement() = default;
        Statement(const Statement &) = delete;
        auto operator=(const Statement &) -> Statement & = delete;
        Statement(Statement &&) = delete;
        auto operator=(Statement &&) -> Statement & = delete;
        virtual ~Statement() = default;

        [[nodiscard]] virtual auto to_string() const noexcept -> std::string = 0;
    };

    using StmtPtr = std::unique_ptr<Statement>;

    struct ExpressionStatement : Statement {
        ExprPtr expression;

        explicit ExpressionStatement(ExprPtr expr) noexcept : expression(std::move(expr)) {
            spdlog::trace("AST: Created ExpressionStatement");
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            return std::format("expr-stmt{{ expr: {} }}", expression->to_string());
        }
    };

    struct PrintStatement : Statement {
        ExprPtr expression;

        explicit PrintStatement(ExprPtr expr) noexcept : expression(std::move(expr)) {
            spdlog::trace("AST: Created PrintStatement");
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            return std::format("print-stmt{{ expr: {} }}", expression->to_string());
        }
    };

    struct VarStatement : Statement {
        syntax::Token name;
        ExprPtr initializer;

        explicit VarStatement(syntax::Token n, ExprPtr init = nullptr) noexcept
            : name(n), initializer(std::move(init)) {
            spdlog::debug("AST: Created VarStatement for variable '{}'", n.lexeme);
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            return std::format("var-stmt{{ name: {}, initializer: {} }}", name.to_string(),
                               initializer ? initializer->to_string() : "null");
        }
    };

    struct BlockStatement : Statement {
        std::vector<StmtPtr> statements;

        explicit BlockStatement(std::vector<StmtPtr> stmts) noexcept : statements(std::move(stmts)) {
            spdlog::debug("AST: Created BlockStatement with {} statements", statements.size());
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            std::string stmts_str;
            for (const auto &stmt : statements) {
                if (!stmts_str.empty())
                    stmts_str += ", ";
                stmts_str += stmt->to_string();
            }
            return std::format("block-stmt{{ statements: [{}] }}", stmts_str);
        }
    };

    struct IfStatement : Statement {
        ExprPtr condition;
        StmtPtr then_branch;
        StmtPtr else_branch;

        IfStatement(ExprPtr cond, StmtPtr then_stmt, StmtPtr else_stmt = nullptr) noexcept
            : condition(std::move(cond)), then_branch(std::move(then_stmt)), else_branch(std::move(else_stmt)) {
            spdlog::debug("AST: Created IfStatement {}else branch", else_branch ? "with " : "without ");
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            return std::format("if-stmt{{ condition: {}, then: {}, else: {} }}", condition->to_string(),
                               then_branch->to_string(), else_branch ? else_branch->to_string() : "null");
        }
    };

    struct WhileStatement : Statement {
        ExprPtr condition;
        StmtPtr body;

        WhileStatement(ExprPtr cond, StmtPtr stmt) noexcept : condition(std::move(cond)), body(std::move(stmt)) {
            spdlog::debug("AST: Created WhileStatement");
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            return std::format("while-stmt{{ condition: {}, body: {} }}", condition->to_string(), body->to_string());
        }
    };

    struct ForStatement : Statement {
        StmtPtr initializer;
        ExprPtr condition;
        ExprPtr increment;
        StmtPtr body;

        ForStatement(StmtPtr init, ExprPtr cond, ExprPtr inc, StmtPtr stmt) noexcept
            : initializer(std::move(init)), condition(std::move(cond)), increment(std::move(inc)),
              body(std::move(stmt)) {
            spdlog::debug("AST: Created ForStatement");
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            return std::format("for-stmt{{ init: {}, condition: {}, increment: {}, body: {} }}",
                               initializer ? initializer->to_string() : "null",
                               condition ? condition->to_string() : "null", increment ? increment->to_string() : "null",
                               body->to_string());
        }
    };

    struct ReturnStatement : Statement {
        syntax::Token keyword;
        ExprPtr value;

        explicit ReturnStatement(syntax::Token kw, ExprPtr val = nullptr) noexcept
            : keyword(std::move(kw)), value(std::move(val)) {
            spdlog::debug("AST: Created ReturnStatement {}return value", value ? "with " : "without ");
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            return std::format("return-stmt{{ value: {} }}", value ? value->to_string() : "null");
        }
    };

    struct FunctionDeclarationStatement : Statement {
        syntax::Token name;
        std::vector<syntax::Token> parameters;
        std::vector<StmtPtr> body;

        FunctionDeclarationStatement(syntax::Token n, std::vector<syntax::Token> params,
                                     std::vector<StmtPtr> stmts) noexcept
            : name(n), parameters(std::move(params)), body(std::move(stmts)) {
            spdlog::info("AST: Created FunctionDeclarationStatement '{}' with {} "
                         "parameters and {} statements",
                         n.lexeme, parameters.size(), body.size());
        }

        [[nodiscard]] auto to_string() const noexcept -> std::string override {
            std::string params_str;
            for (const auto &param : parameters) {
                if (!params_str.empty())
                    params_str += ", ";
                params_str += param.lexeme;
            }

            std::string body_str;
            for (const auto &stmt : body) {
                if (!body_str.empty())
                    body_str += ", ";
                body_str += stmt->to_string();
            }

            return std::format("fun-decl-stmt{{ name: {}, params: [{}], body: [{}] }}", name.to_string(), params_str,
                               body_str);
        }
    };

} // namespace lox::ast

#endif
