#ifndef JACY_IDENTIFIER_H
#define JACY_IDENTIFIER_H

#include <vector>

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Identifier;
    using id_ptr = std::shared_ptr<Identifier>;
    using opt_id_ptr = dt::Option<id_ptr>;
    using IdList = std::vector<id_ptr>;

    struct Identifier : Expr {
        explicit Identifier(parser::opt_token token, const Span & span)
            : token(std::move(token)), Expr(span, ExprKind::Id) {}

        parser::opt_token token;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }

        std::string getValue() const {
            return token->val;
        }
    };
}

#endif // JACY_IDENTIFIER_H
