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
        PathExprSeg(id_ptr id, opt_type_params typeParams, const Location & loc)
            : id(std::move(id)), typeParams(std::move(typeParams)), Node(loc) {}

        id_ptr id;
        opt_type_params typeParams;
    };

    struct PathExpr : Expr {
        PathExpr(bool global, path_expr_list segments, const Location & loc)
            : global(global), segments(std::move(segments)), Expr(loc, ExprType::Path) {}

        bool global;
        path_expr_list segments;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_PATHEXPR_H
