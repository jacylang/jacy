#ifndef JACY_HIR_NODES_TYPES_H
#define JACY_HIR_NODES_TYPES_H

#include "hir/nodes/Type.h"

namespace jc::hir {
    struct TupleType : Type {
        TupleType(type_list && types, const HirId & hirId, const Span & span)
            : Type(TypeKind::Tuple, hirId, span), types(std::move(types)) {}

        type_list types;
    };
}

#endif // JACY_HIR_NODES_TYPES_H
