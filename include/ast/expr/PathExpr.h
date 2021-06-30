#ifndef JACY_AST_EXPR_PATHEXPR_H
#define JACY_AST_EXPR_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct PathExpr;
    struct PathExprSeg;
    using path_expr_ptr = PR<std::shared_ptr<PathExpr>>;
    using path_expr_seg_ptr = PR<std::shared_ptr<PathExprSeg>>;
    using path_expr_seg_list = std::vector<path_expr_seg_ptr>;

    struct PathExprSeg : Node {
        const enum class Kind {
            Super,
            Self,
            Party,
            Ident,

            Error,
        } kind;

        PathExprSeg(
            id_ptr ident,
            opt_gen_params typeParams,
            const Span & span
        ) : Node(span),
            kind(Kind::Ident),
            ident(std::move(ident)),
            typeParams(std::move(typeParams)) {}

        PathExprSeg(
            Kind kind,
            opt_gen_params typeParams,
            const Span & span
        ) : Node(span),
            kind(kind),
            ident(dt::None),
            typeParams(std::move(typeParams)) {}

        opt_id_ptr ident;
        opt_gen_params typeParams;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct PathExpr : Expr {
        PathExpr(bool global, path_expr_seg_list && segments, const Span & span)
            : Expr(span, ExprKind::Path), global(global), segments(std::move(segments)) {}

        bool global;
        path_expr_seg_list segments;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_PATHEXPR_H
