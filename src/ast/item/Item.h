#ifndef JACY_AST_ITEM_ITEM_H
#define JACY_AST_ITEM_ITEM_H

#include "ast/fragments/Attr.h"

namespace jc::ast {
    enum class VisKind {
        Unset,
        Pub,
    };

    /// Visibility (`pub` or unset)
    // TODO: Don't use optional span, try to figure out where user would place `pub` or not to make suggestions better.
    struct Vis {
        Vis() : kind {VisKind::Unset}, span {None} {}

        Vis(VisKind kind, const span::Span::Opt & span) : kind {kind}, span {span} {}

        VisKind kind;
        span::Span::Opt span;
    };

    struct ItemKind {
        using Ptr = PR<N<ItemKind>>;

        enum class Kind {
            Enum,
            Func,
            Impl,
            Init,
            Mod,
            Struct,
            Trait,
            TypeAlias,
            Use,
        };

        ItemKind(Kind kind) : kind {kind} {}
        virtual ~ItemKind() = default;

        Kind kind;

        template<class T>
        static T * as(const N<ItemKind> & item) {
            return static_cast<T *>(item.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct AssocItemKind {
        using Ptr = PR<N<AssocItemKind>>;

        enum class Kind {
            Const,
            Func,
            TypeAlias,
        };

        AssocItemKind(Kind kind) : kind {kind} {}
        virtual ~AssocItemKind() = default;

        Kind kind;

        template<class T>
        static T * as(const N<ItemKind> & item) {
            return static_cast<T *>(item.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    template<class KindT>
    struct _Item : Node {
        using List = std::vector<_Item>;

        _Item(Attr::List && attributes, Vis vis, KindT && kind)
            : Node {span},
              attributes {std::move(attributes)},
              vis {vis},
              kind {std::move(kind)} {}

        Attr::List attributes;
        Vis vis;
        KindT kind;

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct Item : _Item<ItemKind::Ptr> {
        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    using AssocItem = _Item<AssocItemKind>;
}

#endif // JACY_AST_ITEM_ITEM_H
