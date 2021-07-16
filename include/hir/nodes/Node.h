#ifndef JACY_HIR_NODES_NODE_H
#define JACY_HIR_NODES_NODE_H

#include "span/Span.h"

namespace jc::hir {
    struct Node {
        Node(const span::Span & span) : span(span) {}

        span::Span span;
    };
}

#endif // JACY_HIR_NODES_NODE_H
