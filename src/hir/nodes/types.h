#ifndef JACY_HIR_NODES_TYPES_H
#define JACY_HIR_NODES_TYPES_H

#include "hir/nodes/Type.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct TupleType : Type {
        TupleType(Type::List && types, HirId hirId, Span span)
            : Type {Type::Kind::Tuple, hirId, span}, types {std::move(types)} {}

        Type::List types;
    };

    struct FuncType : Type {
        FuncType(Type::List && inputs, Type::Ptr && ret, HirId hirId, Span span)
            : Type {Type::Kind::Func, hirId, span}, inputs {std::move(inputs)}, ret {std::move(ret)} {}

        Type::List inputs;
        Type::Ptr ret;
    };

    struct SliceType : Type {
        SliceType(Type::Ptr && type, HirId hirId, Span span)
            : Type {Type::Kind::Slice, hirId, span}, type {std::move(type)} {}

        Type::Ptr type;
    };

    struct ArrayType : Type {
        ArrayType(Type::Ptr && type, AnonConst && size, HirId hirId, Span span)
            : Type {Type::Kind::Array, hirId, span}, type {std::move(type)}, size {std::move(size)} {}

        Type::Ptr type;
        AnonConst size;
    };

    struct TypePath : Type {
        TypePath(Path && path, HirId hirId, Span span)
            : Type {Type::Kind::Path, hirId, span}, path {std::move(path)} {}

        Path path;
    };
}

#endif // JACY_HIR_NODES_TYPES_H
