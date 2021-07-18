#ifndef JACY_AST_FRAGMENTS_IDENT_H
#define JACY_AST_FRAGMENTS_IDENT_H

#include <vector>

#include "ast/Node.h"
#include "span/Ident.h"

namespace jc::ast {
    struct Ident;
    using ident_pr = PR<Ident>;
    using opt_ident = Option<ident_pr>;

    struct Ident : span::Ident {
        Ident(const std::string & name, const Span & span) : span::Ident(name, span) {}
        Ident(const parser::Token & token) : span::Ident(token) {}

        node_id id;

        void accept(BaseVisitor & visitor) const {
            visitor.visit(*this);
        }

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Ident & ident) {
            os << "Ident(" << ident.name << ")#" << ident.id;
            return os;
        }
    };
}

#endif // JACY_AST_FRAGMENTS_IDENT_H
