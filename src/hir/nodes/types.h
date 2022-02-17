#ifndef JACY_HIR_NODES_TYPES_H
#define JACY_HIR_NODES_TYPES_H

#include "hir/nodes/Type.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct TupleType : TypeKind {
        using Element = NamedNode<Type>;

        TupleType(Element::List && types)
            : TypeKind {TypeKind::Kind::Tuple}, types {std::move(types)} {}

        Element::List types;
    };

    struct FuncType : TypeKind {
        FuncType(Type::List && inputs, Type && ret)
            : TypeKind {TypeKind::Kind::Func}, inputs {std::move(inputs)}, ret {std::move(ret)} {}

        Type::List inputs;
        Type ret;
    };

    struct SliceType : TypeKind {
        SliceType(Type && type)
            : TypeKind {TypeKind::Kind::Slice}, type {std::move(type)} {}

        Type type;
    };

    struct ArrayType : TypeKind {
        ArrayType(Type && type, AnonConst && size)
            : TypeKind {TypeKind::Kind::Array}, type {std::move(type)}, size {std::move(size)} {}

        Type type;
        AnonConst size;
    };

    struct TypePath : TypeKind {
        TypePath(Path && path) : TypeKind {TypeKind::Kind::Path}, path {std::move(path)} {}

        Path path;
    };

    struct UnitType : TypeKind {
        UnitType() : TypeKind {TypeKind::Kind::Unit} {}
    };
}

#endif // JACY_HIR_NODES_TYPES_H
