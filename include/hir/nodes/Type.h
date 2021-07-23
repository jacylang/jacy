#ifndef JACY_HIR_NODES_TYPE_H
#define JACY_HIR_NODES_TYPE_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct Type;
    using type_ptr = std::unique_ptr<Type>;
    using type_list = std::vector<type_ptr>;

    enum class TypeKind {
        Tuple,
        Func,
        Slice,
        Array,
        Path,
    };

    struct Type : HirNode {
        Type(TypeKind kind, const HirId & hirId, const Span & span) : HirNode(hirId, span), kind(kind) {}

        TypeKind kind;
    };
}

#endif // JACY_HIR_NODES_TYPE_H
