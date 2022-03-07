#ifndef JACY_SRC_TYPECK_TYPE_H
#define JACY_SRC_TYPECK_TYPE_H

#include "span/Span.h"
#include "utils/hash.h"

namespace jc::typeck {
    struct Region {
        enum class Kind {
            Static,
            Named,
        };

        Kind kind;

        size_t hash() const {
            return utils::hash::hashEnum(kind);
        }
    };

    struct Mutability {
        enum class Kind {
            Immut, // Default for references, `const` for pointers
            Mut,   // `mut`
        };

        Mutability(Kind kind) : kind {kind} {}

        Kind kind;

        size_t hash() const {
            return utils::hash::hashEnum(kind);
        }
    };

    struct TypeKind {
        using Ptr = std::unique_ptr<TypeKind>;

        /// This is the only list of possible type kinds.
        /// All types are either just primitives or compound of other kinds
        enum class Kind {
            Bottom,

            // Primitive types
            Bool,
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
            Char,
            Str,

            // Compound types
            Ref, // &'a mut T
            Ptr, // *mut T / *const T
            Slice, // [T]
            Array, // [T; n]
        };

        TypeKind(Kind kind) : kind {kind} {}
        virtual ~TypeKind() = default;

        const Kind kind;

        // Each `TypeKind` must implement `hash` function but must not hash its kind as this is done in `Type::hash`
        virtual size_t hash() const = 0;
    };

    /// The main type representation structure
    class Type {
    public:
        Type(TypeKind::Ptr && kind) : kind {std::move(kind)} {}

    public:
        size_t hash() const {
            return utils::hash::hashEnum(kind->kind) + kind->hash();
        }

    private:
        TypeKind::Ptr kind;
    };

    using Ty = std::shared_ptr<Type>;
}

#endif // JACY_SRC_TYPECK_TYPE_H
