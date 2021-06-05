#ifndef JACY_AST_EXPR_LITERALCONSTANT_H
#define JACY_AST_EXPR_LITERALCONSTANT_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct LiteralConstant;
    using literal_ptr = std::shared_ptr<LiteralConstant>;

    struct LiteralConstant : Expr {
        explicit LiteralConstant(const parser::Token & token, const Span & span)
            : token(token), Expr(span, ExprKind::LiteralConstant) {}

        parser::Token token;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LITERALCONSTANT_H
