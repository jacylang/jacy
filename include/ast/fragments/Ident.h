#ifndef JACY_AST_FRAGMENTS_IDENT_H
#define JACY_AST_FRAGMENTS_IDENT_H

#include <vector>

#include "ast/Node.h"
#include "span/Ident.h"

namespace jc::ast {
    using span::Symbol;

    struct Ident : span::Ident {
        using PR = PR<Ident>;
        using OptPR = Option<Ident::PR>;

        Ident(const Symbol & sym, const Span & span) : span::Ident{sym, span} {}
        Ident(const parser::Token & token) : span::Ident{token} {}

        NodeId id{NodeId::DUMMY};

        void accept(BaseVisitor & visitor) const {
            visitor.visit(*this);
        }

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Ident & ident) {
            os << "Ident(" << ident.sym << ")" << ident.id;
            return os;
        }
    };
}

#endif // JACY_AST_FRAGMENTS_IDENT_H
