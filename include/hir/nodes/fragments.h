#ifndef JACY_HIR_NODES_FRAGMENTS_H
#define JACY_HIR_NODES_FRAGMENTS_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct Arg;
    using arg_list = std::vector<Arg>;
    struct Expr;
    using expr_ptr = N<Expr>;

    struct Arg {
        // FIXME: Use `Ident` when it will be unified for both ast and hir
        Arg(const std::string & name, expr_ptr && value, const Span & span)
            : name(name), value(std::move(value)), span(span) {}

        std::string name;
        expr_ptr value;
        Span span;
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
