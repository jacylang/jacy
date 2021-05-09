#ifndef JACY_LITERALCONSTANT_H
#define JACY_LITERALCONSTANT_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct LiteralConstant;
    using literal_ptr = std::shared_ptr<LiteralConstant>;

    struct LiteralConstant : Expr {
        explicit LiteralConstant(const parser::Token & token)
            : token(token), Expr(token.loc, ExprType::LiteralConstant) {}

        parser::Token token;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_LITERALCONSTANT_H
