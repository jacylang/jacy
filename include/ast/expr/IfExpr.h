#ifndef JACY_AST_EXPR_IFEXPR_H
#define JACY_AST_EXPR_IFEXPR_H

#include "ast/expr/Expr.h"

#include <utility>
#include "Block.h"

namespace jc::ast {
    struct IfExpr : Expr {
        IfExpr(
            ExprPtr condition,
            OptBlockPtr ifBranch,
            OptBlockPtr elseBranch,
            const Span & span
        ) : Expr(span, ExprKind::If),
            condition(std::move(condition)),
            ifBranch(std::move(ifBranch)),
            elseBranch(std::move(elseBranch)) {}

        ExprPtr condition;
        OptBlockPtr ifBranch;
        OptBlockPtr elseBranch;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_IFEXPR_H
