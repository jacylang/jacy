#ifndef JACY_AST_FRAGMENTS_IDENT_H
#define JACY_AST_FRAGMENTS_IDENT_H

#include <vector>

#include "ast/Node.h"

namespace jc::ast {
    struct Ident;
    using ident_pr = PR<Ident>;
    using opt_ident_pr = Option<ident_pr>;

    struct Ident : Node {
        explicit Ident(parser::Token token, const Span & span)
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

#endif // JACY_AST_FRAGMENTS_IDENT_H
