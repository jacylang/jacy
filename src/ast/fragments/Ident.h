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

        Ident(Symbol sym, Span span) : span::Ident {sym, span} {}

        Ident(const parser::Token & token) : span::Ident {token} {}

        NodeId id {NodeId::DUMMY};

        void accept(BaseVisitor & visitor) const {
            visitor.visit(*this);
        }

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Ident & ident) {
            os << ident.sym;
            return os;
        }
    };

    template<class T, class Name = Ident::PR>
    struct NamedNode {
        using List = std::vector<NamedNode<T, Name>>;

        NamedNode(Name && name, T && node, Span span)
            : name {std::move(name)}, node {std::move(node)}, span {span} {}

        Name name;
        T node;

        /// Span for the whole node
        Span span;
    };
}

#endif // JACY_AST_FRAGMENTS_IDENT_H
