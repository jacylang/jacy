#ifndef JACY_AST_EXPR_MEMBERACCESS_H
#define JACY_AST_EXPR_MEMBERACCESS_H

#include "ast/expr/Expr.h"
#include "ast/expr/Identifier.h"

namespace jc::ast {
    struct MemberAccess : Expr {
        MemberAccess(
            expr_ptr lhs,
            id_ptr id,
            const Span & span
        ) : Expr(span, ExprType::MemberAccess),
            lhs(std::move(lhs)),
            id(std::move(id)) {}

        expr_ptr lhs;
        id_ptr id;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_EXPR_MEMBERACCESS_H
