#ifndef JACY_AST_EXPR_FIELDEXPR_H
#define JACY_AST_EXPR_FIELDEXPR_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Ident.h"

namespace jc::ast {
    struct FieldExpr : Expr {
        FieldExpr(
            Expr::Ptr && lhs,
            Ident::PR && field,
            const Span & span
        ) : Expr{span, ExprKind::Field},
            lhs{std::move(lhs)},
            field{std::move(field)} {}

        Expr::Ptr lhs;
        Ident::PR field;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_FIELDEXPR_H
