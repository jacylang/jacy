#ifndef JACY_HIR_NODES_NODE_H
#define JACY_HIR_NODES_NODE_H

#include "span/Span.h"

namespace jc::hir {
    using span::Span;

    struct HirNode {
        HirNode(const Span & span) : span(span) {}

        Span span;
    };
}

#endif // JACY_HIR_NODES_NODE_H
