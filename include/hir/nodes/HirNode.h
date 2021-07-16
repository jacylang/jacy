#ifndef JACY_HIR_NODES_HIRNODE_H
#define JACY_HIR_NODES_HIRNODE_H

#include "span/Span.h"
#include "resolve/Definition.h"

namespace jc::hir {
    using span::Span;

    struct HirId {
        resolve::DefId defId;
    };

    struct HirNode {
        HirNode(const Span & span) : span(span) {}

        Span span;
    };
}

#endif // JACY_HIR_NODES_HIRNODE_H
