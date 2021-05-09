#ifndef JACY_IDENTIFIER_H
#define JACY_IDENTIFIER_H

#include <vector>

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Identifier;
    using id_ptr = std::shared_ptr<Identifier>;
    using IdList = std::vector<id_ptr>;

    struct Identifier : Expr {
        Identifier(const parser::Token & token)
            : token(token), Expr(token.loc, ExprType::Id) {}

        parser::Token token;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_IDENTIFIER_H
