#ifndef JACY_HIR_NODES_HIRNODE_H
#define JACY_HIR_NODES_HIRNODE_H

#include "span/Span.h"
#include "resolve/Definition.h"

namespace jc::hir {
    using span::Span;

    template<class T>
    using N = std::unique_ptr<T>;

    struct HirId {
        resolve::DefId defId;
    };

    struct HirNode {
        HirNode(const HirId & hirId, const Span & span) : hirId(hirId), span(span) {}

        HirId hirId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_HIRNODE_H
