#ifndef JACY_AST_EXPR_PATHEXPR_H
#define JACY_AST_EXPR_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Path.h"

namespace jc::ast {
    struct PathExpr;
    using path_expr_ptr = PR<N<PathExpr>>;

    struct PathExpr : Expr {
        PathExpr(path_ptr && path) : Expr(path->span, ExprKind::Path), path(std::move(path)) {}

        path_ptr path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_PATHEXPR_H
