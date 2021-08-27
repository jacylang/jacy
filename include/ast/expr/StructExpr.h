#ifndef AST_EXPR_STRUCTEXPR_H
#define AST_EXPR_STRUCTEXPR_H

#include <variant>

#include "ast/expr/Expr.h"
#include "ast/fragments/Path.h"

namespace jc::ast {
    struct StructExprField : Node {
        using PR = PR<StructExprField>;
        using List = std::vector<PR>;

        enum class Kind {
            Raw, // {field: expr}
            Shortcut, // {field}
            Base, // {...base}
        } kind;

        StructExprField(
            Ident::PR && field,
            Expr::Ptr && expr,
            const Span & span
        ) : Node(span),
            kind(Kind::Raw),
            name(std::move(field)),
            Expr{std::move(expr)} {}

        StructExprField(
            Ident::PR && field,
            const Span & span
        ) : Node(span),
            kind(Kind::Shortcut),
            name(std::move(field)) {}

        StructExprField(
            Expr::Ptr && expr,
            const Span & span
        ) : Node(span),
            kind(Kind::Base),
            Expr{std::move(expr)} {}

        Ident::OptPR name{None};
        Expr::OptPtr expr{None};

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct StructExpr : Expr {
        StructExpr(
            PathExpr::Ptr && path,
            StructExprField::List && fields,
            const Span & span
        ) : Expr{span, ExprKind::Struct},
            path(std::move(path)),
            fields(std::move(fields)) {}

        PathExpr::Ptr path;
        StructExprField::List fields;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // AST_EXPR_STRUCTEXPR_H