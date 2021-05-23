#ifndef JACY_LOOPEXPR_H
#define JACY_LOOPEXPR_H

#include "ast/expr/Expr.h"

#include <utility>
#include "Block.h"

namespace jc::ast {
    struct LoopExpr : Expr {
        LoopExpr(block_ptr body, const Span & span)
            : body(std::move(body)), Expr(span, ExprKind::Loop) {}

        block_ptr body;


        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_LOOPEXPR_H
