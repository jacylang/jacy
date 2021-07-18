#ifndef JACY_SPAN_IDENT_H
#define JACY_SPAN_IDENT_H

#include "span/Span.h"
#include "parser/Token.h"

namespace jc::span {
    struct Ident {
        Ident(const parser::Token & token) {
            if (token.kind != parser::TokenKind::Id) {
                throw std::logic_error("Unexpected kind of token for `Ident` constructor");
            }
            name = token.val;
            span = token.span;
        }
        Ident(const std::string & name, const Span & span) : name(name), span(span) {}

        std::string name;
        Span span;

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Ident & ident) {
            os << "Ident(" << ident.name + ")";
            return os;
        }
    };
}

#endif // JACY_SPAN_IDENT_H
