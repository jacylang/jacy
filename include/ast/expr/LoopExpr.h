#ifndef JACY_LOOPEXPR_H
#define JACY_LOOPEXPR_H

#include "ast/expr/Expr.h"
#include "Block.h"

namespace jc::ast {
    struct LoopExpr : Expr {
        LoopExpr(block_ptr body, const Span & span) : body(body), Expr(span, ExprType::Loop) {}

        block_ptr body;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_LOOPEXPR_H
