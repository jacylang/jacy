#ifndef JACY_HIR_NODES_FRAGMENTS_H
#define JACY_HIR_NODES_FRAGMENTS_H

#include "hir/nodes/HirNode.h"
#include "span/Ident.h"

namespace jc::hir {
    struct Arg;
    using arg_list = std::vector<Arg>;
    struct Expr;
    using expr_ptr = N<Expr>;

    struct Arg {
        Arg(const span::Ident & ident, expr_ptr && value, const Span & span)
            : ident(ident), value(std::move(value)), span(span) {}

        span::Ident ident;
        expr_ptr value;
        Span span;
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
