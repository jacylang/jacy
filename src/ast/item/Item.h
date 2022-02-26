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

    struct ItemKind : Node {
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

        ItemKind(Kind kind, Span span) : Node {span}, kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const N<ItemKind> & item) {
            return static_cast<T *>(item.get());
        }
    };

    template<class KindT = ItemKind>
    struct Item : Node {
        using List = std::vector<Ptr>;

        Item(Attr::List && attributes, Vis vis, Ident::PR && name, KindT && kind)
            : Node {span},
              attributes {std::move(attributes)},
              vis {vis}, name {std::move(name)},
              kind {std::move(kind)} {}

        Attr::List attributes;
        Vis vis;
        Ident::PR name;
        KindT kind;

        virtual span::Ident getName() const = 0;

        virtual NodeId::Opt getNameNodeId() const = 0;

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_ITEM_ITEM_H
