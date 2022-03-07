#ifndef JACY_SRC_TYPECK_TYPE_TYPES_H
#define JACY_SRC_TYPECK_TYPE_TYPES_H

#include "typeck/type/Type.h"

namespace jc::typeck {
    struct Ref : TypeKind {
        Ref(Region region, Ty && type) : TypeKind {TypeKind::Kind::Ref}, region {region}, type {std::move(type)} {}

        Region region;
        Ty type;

        size_t hash() const override {
            return region.hash() ^ type->hash();
        }
    };

    struct Pointer : TypeKind {
        Pointer(Mutability mutability, Ty && type)
            : TypeKind {TypeKind::Kind::Ptr},
              mutability {mutability},
              type {std::move(type)} {}

        Mutability mutability;
        Ty type;

        size_t hash() const override {
            return mutability.hash() ^ type->hash();
        }
    };

    struct Slice : TypeKind {
        Slice(Ty && type) : TypeKind {TypeKind::Kind::Slice}, type {std::move(type)} {}

        Ty type;

        size_t hash() const override {
            return type->hash();
        }
    };

    struct Array : TypeKind {
        Array(Ty && type) : TypeKind {TypeKind::Kind::Array}, type {std::move(type)} {}

        Ty type;
        // TODO!: Const

        size_t hash() const override {
            return type->hash();
        }
    };
}

#endif // JACY_SRC_TYPECK_TYPE_TYPES_H
