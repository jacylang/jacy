#ifndef JACY_AST_EXPR_PATHEXPR_H
#define JACY_AST_EXPR_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Path.h"

namespace jc::ast {
    struct PathExpr;
    using path_expr_ptr = N<PathExpr>;

    struct PathExpr : Expr {
        PathExpr(Path && path, const Span & span) : Expr(span, ExprKind::Path), path(std::move(path)) {}

        Path path;
    };
}

#endif // JACY_AST_EXPR_PATHEXPR_H
