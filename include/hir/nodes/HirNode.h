#ifndef JACY_HIR_NODES_HIRNODE_H
#define JACY_HIR_NODES_HIRNODE_H

#include "span/Span.h"
#include "resolve/Definition.h"

namespace jc::hir {
    using span::Span;
    using resolve::DefId;

    template<class T>
    using N = std::unique_ptr<T>;

    struct OwnerDef {
        using IdT = uint32_t;

        OwnerDef(DefId defId, IdT initialId) : defId{defId}, nextId{initialId} {}

        DefId defId;
        IdT nextId;
    };

    struct HirId {
        HirId(const resolve::DefId & defId, const OwnerDef::IdT & id) : defId{defId}, id{id} {}

        static const HirId DUMMY;

        resolve::DefId defId;
        OwnerDef::IdT id;
    };

    struct HirNode {
        HirNode(const HirId & hirId, const Span & span) : hirId{hirId}, span{span} {}

        HirId hirId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_HIRNODE_H
