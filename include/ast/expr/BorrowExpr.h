#ifndef JACY_AST_EXPR_BORROWEXPR_H
#define JACY_AST_EXPR_BORROWEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct BorrowExpr : Expr {
        BorrowExpr(
            bool twin,
            bool mut,
            expr_ptr expr,
            const Span & span
        ) : Expr(span, ExprKind::Borrow),
            twin(twin),
            mut(mut),
            expr(std::move(expr)) {}

        // TODO: Bool storing optimization
        bool twin;
        bool mut;
        expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(const ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_BORROWEXPR_H
