#ifndef JACY_AST_EXPR_PATHEXPR_H
#define JACY_AST_EXPR_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Path.h"

namespace jc::ast {
    struct PathExpr;
    using path_expr_ptr = PR<N<PathExpr>>;

    struct PathExpr : Expr {
        PathExpr(Path && path, const Span & span) : Expr(span, ExprKind::Path), path(std::move(path)) {}

        Path path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_PATHEXPR_H
