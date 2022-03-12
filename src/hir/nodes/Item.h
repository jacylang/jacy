#ifndef JACY_HIR_NODES_ITEM_H
#define JACY_HIR_NODES_ITEM_H

#include "resolve/Definition.h"

namespace jc::hir {
    using resolve::DefId;

    struct ItemId {
        template<class T>
        using Map = std::map<ItemId, T>;
        using Opt = Option<ItemId>;
        using List = std::vector<ItemId>;

        resolve::DefId defId;

        bool operator<(const ItemId & other) const {
            return defId < other.defId;
        }
    };

    struct TraitMemberId {
        template<class T>
        using Map = std::map<TraitMemberId, T>;
        using Opt = Option<TraitMemberId>;
        using List = std::vector<TraitMemberId>;

        resolve::DefId defId;

        bool operator<(const TraitMemberId & other) const {
            return defId < other.defId;
        }
    };

    struct ImplMemberId {
        template<class T>
        using Map = std::map<ImplMemberId, T>;
        using Opt = Option<ImplMemberId>;
        using List = std::vector<ImplMemberId>;

        resolve::DefId defId;

        bool operator<(const ImplMemberId & other) const {
            return defId < other.defId;
        }
    };

    /// The base class for all items
    struct ItemKind {
        using Ptr = std::unique_ptr<ItemKind>;

        enum class Kind {
            Const,
            Enum,
            Func,
            Impl,
            Mod,
            Struct,
            Trait,
            TypeAlias,
            Use,
        };

        ItemKind(Kind kind) : kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & item) {
            return static_cast<T *>(item.get());
        }
    };

    /// The wrapper over `Item` and its additional info.
    /// It is useful because we can lower specific item independently and then construct the full `Item`.
    struct Item {
        using Vis = ast::Vis;

        // Special structure for data stored outside `ItemKind` kinds but may be passed with it to visitors
        struct ItemData {
            Vis vis;
            Ident name;
            DefId defId;
            NodeId nodeId;
        };

        Item(Vis vis, Ident && name, ItemKind::Ptr && kind, DefId defId, NodeId nodeId, Span span)
            : vis {vis},
              name {std::move(name)},
              kind {std::move(kind)},
              defId {defId},
              nodeId {nodeId},
              span {span} {}

        Vis vis;
        Ident name;
        ItemKind::Ptr kind;
        DefId defId;
        NodeId nodeId;
        Span span;

        ItemData getItemData() const {
            return ItemData {
                vis,
                name,
                defId,
                nodeId,
            };
        }
    };
}

#endif // JACY_HIR_NODES_ITEM_H
