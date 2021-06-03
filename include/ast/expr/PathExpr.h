#ifndef JACY_AST_EXPR_PATHEXPR_H
#define JACY_AST_EXPR_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/TypeParams.h"

namespace jc::ast {
    struct PathExpr;
    struct PathExprSeg;
    using path_expr_ptr = PR<std::shared_ptr<PathExpr>>;
    using path_expr_seg_ptr = std::shared_ptr<PathExprSeg>;
    using path_expr_seg_list = std::vector<path_expr_seg_ptr>;

    struct PathExprSeg : Node {
        const enum class Kind {
            Super,
            Self,
            Party,
            Ident,
        } kind;

        PathExprSeg(id_ptr ident, opt_type_params typeParams, const Span & span)
            : ident(std::move(ident)), kind(Kind::Ident), typeParams(std::move(typeParams)), Node(span) {}

        PathExprSeg(Kind kind, opt_type_params typeParams, const Span & span)
            : ident(dt::None), kind(kind), typeParams(std::move(typeParams)), Node(span) {}

        opt_id_ptr ident;
        opt_type_params typeParams;
    };

    struct PathExpr : Expr {
        PathExpr(bool global, path_expr_seg_list && segments, const Span & span)
            : global(global), segments(std::move(segments)), Expr(span, ExprKind::Path) {}

        bool global;
        path_expr_seg_list segments;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_PATHEXPR_H
