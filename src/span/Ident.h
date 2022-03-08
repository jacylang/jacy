#ifndef JACY_SPAN_IDENT_H
#define JACY_SPAN_IDENT_H

#include "span/Span.h"
#include "parser/Token.h"

namespace jc::span {
    struct Ident {
        using List = std::vector<Ident>;
        using Opt = Option<Ident>;

        Ident(const parser::Token & token) {
            if (token.kind != parser::TokenKind::Id) {
                throw std::logic_error("Unexpected kind of token for `Ident` constructor");
            }
            sym = token.asSymbol();
            span = token.span;
        }
        Ident(Symbol sym, Span span) : sym{sym}, span{span} {}

        Symbol sym;
        Span span;

        static Ident empty() {
            static const Ident ident{Symbol::empty(), NONE_SPAN};
            return ident;
        }

        size_t hash() const {
            return sym.id.val;
        }

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Ident & ident) {
            os << ident.sym;
            return os;
        }
    };

    template<class T>
    struct Named {
        using Opt = Option<Named>;
        using List = std::vector<Named>;

        Ident name;
        T value;
    };
}

#endif // JACY_SPAN_IDENT_H
