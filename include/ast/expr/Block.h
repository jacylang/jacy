#ifndef JACY_AST_EXPR_BLOCK_H
#define JACY_AST_EXPR_BLOCK_H

#include "ast/expr/Expr.h"
#include "ast/stmt/Stmt.h"

namespace jc::ast {
    struct Block;
    using block_ptr = PR<N<Block>>;
    using opt_block_ptr = Option<block_ptr>;
    using block_list = std::vector<block_ptr>;

    enum class BlockKind {
        OneLine,
        Raw,
    };

    struct Block : Expr {
        Block(expr_ptr && expr, const Span & span)
            : Expr(span, ExprKind::Block),
              blockKind(BlockKind::OneLine),
              oneLine(std::move(expr)) {}

        Block(stmt_list && stmts, const Span & span)
            : Expr(span, ExprKind::Block),
              blockKind(BlockKind::Raw),
              stmts(std::move(stmts)) {}

        BlockKind blockKind;
        opt_expr_ptr oneLine{None};
        Option<stmt_list> stmts{None};

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_BLOCK_H
