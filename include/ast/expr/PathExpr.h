#ifndef JACY_AST_EXPR_PATHEXPR_H
#define JACY_AST_EXPR_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Path.h"

namespace jc::ast {
    struct PathExpr : Expr {
        PathExpr(path_ptr && path, const Span & span) : Expr(span, ExprKind::Path), path(std::move(path)) {}

        path_ptr path;
    };
}

#endif // JACY_AST_EXPR_PATHEXPR_H
