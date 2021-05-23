#ifndef JACY_IDENTIFIER_H
#define JACY_IDENTIFIER_H

#include <vector>

#include "ast/Node.h"

namespace jc::ast {
    struct Identifier;
    using id_ptr = std::shared_ptr<Identifier>;
    using opt_id_ptr = dt::Option<id_ptr>;

    struct Identifier : Node {
        explicit Identifier(parser::opt_token token, const Span & span)
            : token(token), Node(span) {}

        parser::opt_token token;

        dt::Option<std::string> getValue() const {
            if (token) {
                return token->val;
            }
            return dt::None;
        }

        std::string unwrapValue() const {
            if (token) {
                return token->val;
            }
            common::Logger::devPanic("Called `Identifier::unwrapValue` on [ERROR ID]");
        }

        void setReference(node_id reference) {
            refersTo = reference;
        }

    private:
        node_id refersTo;
    };
}

#endif // JACY_IDENTIFIER_H
