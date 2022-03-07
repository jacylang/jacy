#ifndef JACY_SRC_TYPECK_TYPE_H
#define JACY_SRC_TYPECK_TYPE_H

#include "span/Span.h"

namespace jc::typeck {
    struct Region {
        enum class Kind {
            Static,
            Named,
        };

        Kind kind;
    };

    enum class Mutability {
        Immut, // Default for references, `const` for pointers
        Mut,   // `mut`
    };

    struct TypeKind {
        using Ptr = std::unique_ptr<TypeKind>;

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

        const Kind kind;
    };

    /// The main type representation structure
    class Type {
    public:
        Type(TypeKind && kind) : kind {std::move(kind)} {}

    private:
        TypeKind kind;
    };
}

#endif // JACY_SRC_TYPECK_TYPE_H
