#ifndef JACY_SRC_TYPECK_TYPE_H
#define JACY_SRC_TYPECK_TYPE_H

#include "span/Span.h"
#include "utils/hash.h"
#include "resolve/Definition.h"

namespace jc::typeck {
    using resolve::DefId;
    using span::Ident;

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
            Infer,

            // Primitive types
            Bool,
            Char,
            Int,
            Float,
            Str,

            // Compound types
            Ref, // &'a mut T
            Ptr, // *mut T / *const T
            Slice, // [T]
            Array, // [T; n]
            Tuple,
            Unit,

            // Item types
            Func,
        };

        TypeKind(Kind kind) : kind {kind} {}

        virtual ~TypeKind() = default;

        const Kind kind;

        // Each `TypeKind` must implement `hash` function but must not hash its kind as this is done in `Type::hash`
        virtual size_t hash() const = 0;

        template<class T, class ...Args>
        static Ptr make(Args && ...args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        template<class T>
        static T * as(const Ptr & type) {
            return static_cast<T*>(type.get());
        }
    };

    /// The main type representation structure
    class Type {
    public:
        using Ptr = std::shared_ptr<Type>;
        using List = std::vector<Ptr>;

    public:
        Type(TypeKind::Ptr && kind) : kind {std::move(kind)} {}

        size_t hash() const {
            return hashKind(kind);
        }

        static size_t hashKind(const TypeKind::Ptr & kind) {
            return utils::hash::hashEnum(kind->kind) + kind->hash();
        }

        TypeKind::Ptr kind;
    };

    using Ty = Type::Ptr;
}

#endif // JACY_SRC_TYPECK_TYPE_H
