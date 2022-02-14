#ifndef JACY_HIR_NODES_TYPES_H
#define JACY_HIR_NODES_TYPES_H

#include "hir/nodes/Type.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct TupleType : TypeKind {
        TupleType(Type::List && types, Span span)
            : TypeKind {TypeKind::Kind::Tuple}, types {std::move(types)} {}

        Type::List types;
    };

    struct FuncType : TypeKind {
        FuncType(Type::List && inputs, Type && ret, Span span)
            : TypeKind {TypeKind::Kind::Func}, inputs {std::move(inputs)}, ret {std::move(ret)} {}

        Type::List inputs;
        Type ret;
    };

    struct SliceType : TypeKind {
        SliceType(Type && type, Span span)
            : TypeKind {TypeKind::Kind::Slice}, type {std::move(type)} {}

        Type type;
    };

    struct ArrayType : TypeKind {
        ArrayType(Type && type, AnonConst && size, Span span)
            : TypeKind {TypeKind::Kind::Array}, type {std::move(type)}, size {std::move(size)} {}

        Type type;
        AnonConst size;
    };

    struct TypePath : TypeKind {
        TypePath(Path && path, Span span)
            : TypeKind {TypeKind::Kind::Path}, path {std::move(path)} {}

        Path path;
    };

    struct UnitType : TypeKind {
        UnitType(Span span) : TypeKind {TypeKind::Kind::Unit} {}
    };
}

#endif // JACY_HIR_NODES_TYPES_H
