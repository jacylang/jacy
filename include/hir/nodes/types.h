#ifndef JACY_HIR_NODES_TYPES_H
#define JACY_HIR_NODES_TYPES_H

#include "hir/nodes/Type.h"
#include "hir/nodes/Expr.h"

namespace jc::hir {
    struct TupleType : Type {
        TupleType(Type::List && types, const HirId & hirId, const Span & span)
            : Type(TypeKind::Tuple, hirId, span), types(std::move(types)) {}

        Type::List types;
    };

    struct FuncType : Type {
        FuncType(Type::List && inputs, type_ptr && ret, const HirId & hirId, const Span & span)
            : Type(TypeKind::Func, hirId, span), inputs(std::move(inputs)), ret(std::move(ret)) {}

        Type::List inputs;
        type_ptr ret;
    };

    struct SliceType : Type {
        SliceType(type_ptr && type, const HirId & hirId, const Span & span)
            : Type(TypeKind::Slice, hirId, span), type(std::move(type)) {}

        type_ptr type;
    };

    struct ArrayType : Type {
        ArrayType(type_ptr && type, expr_ptr && size, const HirId & hirId, const Span & span)
            : Type(TypeKind::Array, hirId, span), type(std::move(type)), size(std::move(size)) {}

        type_ptr type;
        expr_ptr size;
    };

    struct TypePath : Type {
        TypePath(Path && path, const HirId & hirId, const Span & span)
            : Type(TypeKind::Path, hirId, span), path(std::move(path)) {}

        Path path;
    };
}

#endif // JACY_HIR_NODES_TYPES_H
