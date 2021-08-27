#ifndef JACY_AST_EXPR_PATHEXPR_H
#define JACY_AST_EXPR_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Path.h"

namespace jc::ast {
    struct PathExpr;
    using PathExprPtr = PR<N<PathExpr>>;

    struct PathExpr : Expr {
        PathExpr(Path && path) : Expr(path.span, ExprKind::Path), path(std::move(path)) {}

        Path path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_PATHEXPR_H
