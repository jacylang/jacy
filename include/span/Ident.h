#ifndef JACY_SPAN_IDENT_H
#define JACY_SPAN_IDENT_H

#include "span/Span.h"
#include "parser/Token.h"

namespace jc::span {
    struct Ident {
        using Opt = Option<Ident>;

        Ident(const parser::Token & token) {
            if (token.kind != parser::TokenKind::Id) {
                throw std::logic_error("Unexpected kind of token for `Ident` constructor");
            }
            sym = token.asSymbol();
            span = token.span;
        }
        Ident(const Symbol & sym, const Span & span) : sym{sym}, span{span} {}

        Symbol sym;
        Span span;

        static Ident empty() {
            static const Ident ident{"", NONE_SPAN};
            return ident;
        }

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Ident & ident) {
            os << "Ident(" << ident.sym + ")";
            return os;
        }
    };
}

#endif // JACY_SPAN_IDENT_H
