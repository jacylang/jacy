#ifndef JACY_SRC_TYPECK_TYPE_TYPES_H
#define JACY_SRC_TYPECK_TYPE_TYPES_H

#include "typeck/type/Type.h"

namespace jc::typeck {
    struct Bottom : TypeKind {
        Bottom() : TypeKind {TypeKind::Kind::Bottom} {}

        size_t hash() const override {
            return 0;
        }
    };

    struct Infer : TypeKind {
        Infer() : TypeKind {TypeKind::Kind::Infer} {}

        size_t hash() const override {
            return 0;
        }
    };

    struct Bool : TypeKind {
        Bool() : TypeKind {TypeKind::Kind::Bool} {}

        size_t hash() const override {
            return 0;
        }
    };

    struct Char : TypeKind {
        Char() : TypeKind {TypeKind::Kind::Char} {}

        size_t hash() const override {
            return 0;
        }
    };

    struct Int : TypeKind {
        enum class Kind {
            Int,
            Uint,
            I8,
            I16,
            I32,
            I64,
            U8,
            U16,
            U32,
            U64,
        };

        Int(Kind kind) : TypeKind {TypeKind::Kind::Int}, kind {kind} {}

        Kind kind;

        size_t hash() const override {
            return 0;
        }
    };

    struct Float : TypeKind {
        enum class Kind {
            F32,
            F64,
        };

        Float(Kind kind) : TypeKind {TypeKind::Kind::Float}, kind {kind} {}

        Kind kind;

        size_t hash() const override {
            return 0;
        }
    };

    struct Str : TypeKind {
        Str() : TypeKind {TypeKind::Kind::Str} {}

        size_t hash() const override {
            return 0;
        }
    };

    struct Ref : TypeKind {
        Ref(Region region, Ty type) : TypeKind {TypeKind::Kind::Ref}, region {region}, type {type} {}

        Region region;
        Ty type;

        size_t hash() const override {
            return region.hash() ^ type->hash();
        }
    };

    struct Pointer : TypeKind {
        Pointer(Mutability mutability, Ty type)
            : TypeKind {TypeKind::Kind::Ptr},
              mutability {mutability},
              type {type} {}

        Mutability mutability;
        Ty type;

        size_t hash() const override {
            return mutability.hash() ^ type->hash();
        }
    };

    struct Slice : TypeKind {
        Slice(Ty type) : TypeKind {TypeKind::Kind::Slice}, type {type} {}

        Ty type;

        size_t hash() const override {
            return type->hash();
        }
    };

    struct Array : TypeKind {
        Array(Ty type) : TypeKind {TypeKind::Kind::Array}, type {type} {}

        Ty type;
        // TODO!: Const

        size_t hash() const override {
            return type->hash();
        }
    };

    struct Tuple : TypeKind {
        using Element = span::Named<Ty>;

        Tuple(Element::List && elements) : TypeKind {TypeKind::Kind::Tuple}, elements {std::move(elements)} {}

        Element::List elements;

        size_t hash() const override {
            size_t hash = 0;
            for (const auto & el : elements) {
                hash ^= el.name.hash() + el.value->hash();
            }
            return hash;
        }
    };

    struct Func : TypeKind {
        Func(DefId defId, Type::List && inputs, Ty output)
            : TypeKind {TypeKind::Kind::Func},
              defId {defId},
              inputs {std::move(inputs)},
              output {output} {}

        DefId defId;

        // TODO: Do we need parameter names? For arguments order check?
        Type::List inputs;
        Ty output;

        size_t hash() const override {
            size_t hash = 0;
            for (const auto & el : inputs) {
                hash ^= el->hash();
            }
            hash ^= output->hash();
            return hash;
        }
    };
}

#endif // JACY_SRC_TYPECK_TYPE_TYPES_H
