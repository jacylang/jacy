#ifndef JACY_HIR_NODES_TYPE_H
#define JACY_HIR_NODES_TYPE_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    enum class TypeKind {
        Tuple,
        Func,
        Slice,
        Array,
        Path,
    };

    struct Type : HirNode {
        using Ptr = std::unique_ptr<Type>;
        using List = std::vector<Type::Ptr>;

        Type(TypeKind kind, const HirId & hirId, const Span & span) : HirNode(hirId, span), kind(kind) {}

        TypeKind kind;
    };
}

#endif // JACY_HIR_NODES_TYPE_H
