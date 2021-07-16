#ifndef JACY_AST_EXPR_LOOPEXPR_H
#define JACY_AST_EXPR_LOOPEXPR_H

#include "ast/expr/Expr.h"

#include <utility>
#include "Block.h"

namespace jc::ast {
    struct LoopExpr : Expr {
        LoopExpr(block_ptr && body, const Span & span)
            : Expr(span, ExprKind::Loop), body(std::move(body)) {}

        block_ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LOOPEXPR_H
