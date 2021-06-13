#ifndef JACY_AST_EXPR_BLOCK_H
#define JACY_AST_EXPR_BLOCK_H

#include "ast/expr/Expr.h"
#include "ast/stmt/Stmt.h"

namespace jc::ast {
    struct Block;
    using block_ptr = std::shared_ptr<Block>;
    using opt_block_ptr = dt::Option<block_ptr>;
    using block_list = std::vector<block_ptr>;

    struct Block : Expr {
        Block(expr_ptr expr, const Span & span)
            : Expr(span, ExprKind::Block), oneLine(std::move(expr)) {}

        Block(stmt_list stmts, const Span & span)
            : Expr(span, ExprKind::Block), stmts(std::move(stmts)) {}

        opt_expr_ptr oneLine;
        dt::Option<stmt_list> stmts;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_BLOCK_H
