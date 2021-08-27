#ifndef JACY_AST_EXPR_MEMBERACCESS_H
#define JACY_AST_EXPR_MEMBERACCESS_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Ident.h"

namespace jc::ast {
    struct MemberAccess : Expr {
        MemberAccess(
            Expr::Ptr && lhs,
            Ident::PR && field,
            const Span & span
        ) : Expr{span, ExprKind::MemberAccess},
            lhs{std::move(lhs)},
            field{std::move(field)} {}

        Expr::Ptr lhs;
        Ident::PR field;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_MEMBERACCESS_H
