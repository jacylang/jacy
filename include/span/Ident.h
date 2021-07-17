#ifndef JACY_SPAN_IDENT_H
#define JACY_SPAN_IDENT_H

#include "span/Span.h"

namespace jc::span {
    struct Ident {
        Ident(const std::string & name, const Span & span) : name(name), span(span) {}

        std::string name;
        Span span;
    };
}

#endif // JACY_SPAN_IDENT_H
