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
        struct Var {
            using ValueT = uint32_t;

            bool operator==(const Var & other) const {
                return val == other.val;
            }

            bool operator<(const Var & other) const {
                return val < other.val;
            }

            Var operator++(int) {
                return Var {val++};
            }

            ValueT val;
        };

        using ValueT = std::variant<Var>;

        enum class Kind {
            Var,
        };

        Infer(Var var) : TypeKind {TypeKind::Kind::Infer}, value {var} {}

        Kind kind;
        ValueT value;

        const Var & asVar() const {
            return std::get<Var>(value);
        }

        size_t hash() const override {
            size_t hash = utils::hash::hashEnum(kind);
            switch (kind) {
                case Kind::Var: {
                    hash ^= asVar().val;
                    break;
                }
            }
            return hash;
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
        Ref(Region region, Mutability mutability, Ty type)
            : TypeKind {TypeKind::Kind::Ref}, region {region}, mutability {mutability}, type {type} {}

        Region region;
        Mutability mutability;
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
        using Element = span::Named<Ty, Ident::Opt>;

        Tuple(Element::List && elements) : TypeKind {TypeKind::Kind::Tuple}, elements {std::move(elements)} {}

        Element::List elements;

        size_t hash() const override {
            size_t hash = 0;
            for (const auto & el : elements) {
                auto elHash = el.value->hash();
                el.name.then([&](const Ident & name) {
                    elHash += name.hash();
                });
                hash ^= elHash;
            }
            return hash;
        }
    };

    struct Unit : TypeKind {
        Unit() : TypeKind {TypeKind::Kind::Unit} {}

        size_t hash() const override {
            return 0;
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
        //  The current answer is No! Because named arguments is a name resolution function overloading mechanism
        //  but not type-dependent.
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
