#ifndef JACY_AST_FRAGMENTS_IDENTIFIER_H
#define JACY_AST_FRAGMENTS_IDENTIFIER_H

#include <vector>

#include "ast/Node.h"

namespace jc::ast {
    struct Identifier;
    using id_ptr = PR<std::shared_ptr<Identifier>>;
    using opt_id_ptr = dt::Option<id_ptr>;

    struct Identifier : Node {
        explicit Identifier(parser::Token token, const Span & span)
            : Node(span), token(token) {}

        parser::Token token;

        std::string getValue() const {
            return token.val;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_IDENTIFIER_H
