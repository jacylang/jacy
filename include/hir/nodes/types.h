#ifndef JACY_HIR_NODES_TYPES_H
#define JACY_HIR_NODES_TYPES_H

#include "hir/nodes/Type.h"

namespace jc::hir {
    struct TupleType : Type {
        TupleType(type_list && types, const HirId & hirId, const Span & span)
            : Type(TypeKind::Tuple, hirId, span), types(std::move(types)) {}

        type_list types;
    };

    struct FuncType : Type {
        FuncType(type_list && inputs, type_ptr && ret, const HirId & hirId, const Span & span)
            : Type(TypeKind::Func, hirId, span), inputs(std::move(inputs)), ret(std::move(ret)) {}

        type_list inputs;
        type_ptr ret;
    };

    struct Slice : Type {
        Slice(type_ptr && type, const HirId & hirId, const Span & span)
            : Type(TypeKind::Slice, hirId, span), type(std::move(type)) {}

        type_ptr type;
    };
}

#endif // JACY_HIR_NODES_TYPES_H
