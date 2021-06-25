#ifndef JACY_AST_EXPR_MEMBERACCESS_H
#define JACY_AST_EXPR_MEMBERACCESS_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Identifier.h"

namespace jc::ast {
    struct MemberAccess : Expr {
        MemberAccess(
            expr_ptr lhs,
            id_ptr field,
            const Span & span
        ) : Expr(span, ExprKind::MemberAccess),
            lhs(std::move(lhs)),
            field(std::move(field)) {}

        MemberAccess(
            parser::Token self,
            id_ptr field,
            const Span & span
        ) : Expr(span, ExprKind::MemberAccess),
            self(self),
            field(std::move(field)) {}

        dt::Option<parser::Token> self{dt::None};
        opt_expr_ptr lhs{dt::None};
        id_ptr field;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_MEMBERACCESS_H
