#ifndef JACY_HIR_NODES_TYPE_H
#define JACY_HIR_NODES_TYPE_H

namespace jc::hir {
    struct TypeKind {
        using Ptr = std::unique_ptr<TypeKind>;

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
        using Opt = Option<Type>;
        using List = std::vector<Type>;

        Type(TypeKind::Ptr && kind, Span span) : span {span}, kind {std::move(kind)} {}

        Span span;
        TypeKind::Ptr kind;
    };
}

#endif // JACY_HIR_NODES_TYPE_H
