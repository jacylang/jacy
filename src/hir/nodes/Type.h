#ifndef JACY_HIR_NODES_TYPE_H
#define JACY_HIR_NODES_TYPE_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct TypeKind {
        using Ptr = N<TypeKind>;
        using OptPtr = Option<Ptr>;

        enum class Kind {
            Infer,
            Tuple,
            Func,
            Slice,
            Array,
            Path,
            Unit,
        };

        TypeKind(Kind kind) : kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & item) {
            return static_cast<T*>(item.get());
        }
    };

    struct Type  {
        using List = std::vector<Type>;

        Type(TypeKind::Ptr && kind, HirId hirId, Span span)
            : hirId {hirId}, span {span}, kind {std::move(kind)} {}

        HirId hirId;
        Span span;
        TypeKind::Ptr kind;
    };
}

#endif // JACY_HIR_NODES_TYPE_H
