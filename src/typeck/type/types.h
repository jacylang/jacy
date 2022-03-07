#ifndef JACY_SRC_TYPECK_TYPE_TYPES_H
#define JACY_SRC_TYPECK_TYPE_TYPES_H

#include "typeck/type/Type.h"

namespace jc::typeck {
    struct Ref : TypeKind {
        Ref(Region region, Type && type) : TypeKind {TypeKind::Kind::Ref}, region {region}, type {std::move(type)} {}

        Region region;
        Type type;
    };

    struct Pointer : TypeKind {
        Pointer(Mutability mutability, Type && type)
            : TypeKind {TypeKind::Kind::Ptr},
              mutability {mutability},
              type {std::move(type)} {}

        Mutability mutability;
        Type type;
    };

    struct Slice : TypeKind {
        Slice(Type && type) : TypeKind {TypeKind::Kind::Slice}, type {std::move(type)} {}

        Type type;
    };

    struct Array : TypeKind {
        Array(Type && type) : TypeKind {TypeKind::Kind::Array}, type {std::move(type)} {}

        Type type;
        // TODO!: C
    };
}

#endif // JACY_SRC_TYPECK_TYPE_TYPES_H
