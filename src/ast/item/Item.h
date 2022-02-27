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
        using Opt = Option<Ptr>;

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

        // TODO!: This NodeId is actually a copy of a id of `Item` wrapper,
        //  as we have visitors and ItemKind visitors do not have access to `Item` NodeId.
        //  Thus, get rid of it when visitors will be removed or something else, idk.
        NodeId id;
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
    struct _Item {
        _Item(Attr::List && attributes, Vis vis, KindT && kind, Span span)
            : attributes {std::move(attributes)},
              vis {vis},
              kind {std::move(kind)},
              span {span} {}
        virtual ~_Item() = default;

        Attr::List attributes;
        Vis vis;
        KindT kind;
        Span span;

        virtual NodeId nodeId() const = 0;
        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct Item : _Item<ItemKind::Ptr> {
        using Opt = Option<Item>;
        using List = std::vector<Item>;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }

        NodeId nodeId() const override {
            return kind.nodeId();
        }
    };

    using AssocItem = _Item<AssocItemKind>;
}

#endif // JACY_AST_ITEM_ITEM_H
