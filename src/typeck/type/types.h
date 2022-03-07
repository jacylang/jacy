#ifndef JACY_SRC_TYPECK_TYPE_TYPES_H
#define JACY_SRC_TYPECK_TYPE_TYPES_H

#include "typeck/type/Type.h"

namespace jc::typeck {
    struct Ref : TypeKind {
        Ref(Region region, Type && type) : TypeKind {TypeKind::Kind::Ref}, region {region}, type {std::move(type)} {}

        Region region;
        Type type;

        size_t hash() const override {
            return region.hash() ^ type.hash();
        }
    };

    struct Pointer : TypeKind {
        Pointer(Mutability mutability, Type && type)
            : TypeKind {TypeKind::Kind::Ptr},
              mutability {mutability},
              type {std::move(type)} {}

        Mutability mutability;
        Type type;

        size_t hash() const override {
            return mutability.hash() ^ type.hash();
        }
    };

    struct Slice : TypeKind {
        Slice(Type && type) : TypeKind {TypeKind::Kind::Slice}, type {std::move(type)} {}

        Type type;

        size_t hash() const override {
            return type.hash();
        }
    };

    struct Array : TypeKind {
        Array(Type && type) : TypeKind {TypeKind::Kind::Array}, type {std::move(type)} {}

        Type type;
        // TODO!: Const

        size_t hash() const override {
            return type.hash();
        }
    };
}

#endif // JACY_SRC_TYPECK_TYPE_TYPES_H
