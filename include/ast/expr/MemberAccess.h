#ifndef JACY_AST_EXPR_MEMBERACCESS_H
#define JACY_AST_EXPR_MEMBERACCESS_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Ident.h"

namespace jc::ast {
    struct MemberAccess : Expr {
        MemberAccess(
            ExprPtr && lhs,
            ident_pr && field,
            const Span & span
        ) : Expr(span, ExprKind::MemberAccess),
            lhs(std::move(lhs)),
            field(std::move(field)) {}

        ExprPtr lhs;
        ident_pr field;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_MEMBERACCESS_H
