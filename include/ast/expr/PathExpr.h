#ifndef JACY_AST_PATHEXPR_H
#define JACY_AST_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/TypeParams.h"

namespace jc::ast {
    struct PathExprSeg;
    using path_expr_ptr = std::shared_ptr<PathExprSeg>;
    using path_expr_list = std::vector<path_expr_ptr>;

    struct PathExprSeg : Node {
        PathExprSeg(id_ptr name, opt_type_params typeParams, const Span & span)
            : name(std::move(name)), typeParams(std::move(typeParams)), Node(span) {}

        id_ptr name;
        opt_type_params typeParams;
    };

    struct PathExpr : Expr {
        PathExpr(bool global, path_expr_list segments, const Span & span)
            : global(global), segments(std::move(segments)), Expr(span, ExprKind::Path) {}

        bool global;
        path_expr_list segments;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_PATHEXPR_H
