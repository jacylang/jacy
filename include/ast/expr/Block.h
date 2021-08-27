#ifndef JACY_AST_EXPR_BLOCK_H
#define JACY_AST_EXPR_BLOCK_H

#include "ast/expr/Expr.h"
#include "ast/stmt/Stmt.h"

namespace jc::ast {
    struct Block : Expr {
        using Ptr = PR<N<Block>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        Block(Stmt::List && stmts, const Span & span)
            : Expr{span, ExprKind::Block},
              stmts(std::move(stmts)) {}

        Stmt::List stmts;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_BLOCK_H
