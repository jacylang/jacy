#ifndef JACY_HIR_NODES_PARTY_H
#define JACY_HIR_NODES_PARTY_H

#include "utils/map.h"
#include "hir/nodes/exprs.h"
#include "hir/nodes/stmts.h"
#include "hir/nodes/items.h"
#include "hir/nodes/types.h"
#include "hir/nodes/patterns.h"

namespace jc::hir {
    using resolve::DefId;

    struct OwnerNode {
        using ValueT = std::variant<Mod, Item>;

        enum class Kind {
            Party,
            Item,
        };

        OwnerNode(Mod && rootMod) : kind {Kind::Party}, value {std::move(rootMod)} {}

        OwnerNode(Item && item) : kind {Kind::Item}, value {std::move(item)} {}

        Kind kind;
        ValueT value;

        const Mod & asParty() const {
            if (kind != Kind::Party) {
                log::devPanic("Called `OwnerNode::asParty` on non-party `OwnerNode`");
            }
            return std::get<Mod>(value);
        }

        const Item & asItem() const {
            if (kind != Kind::Item) {
                log::devPanic("Called `OwnerNode::asItem` on non-item `OwnerNode`");
            }
            return std::get<Item>(value);
        }
    };

    struct HirNode {
        template<class T>
        using N = T*;

        /// HIR nodes types.
        /// It's easy to remember that here are only the nodes that contain `HirId`,
        ///  whereas other nodes are just "fragment" nodes
        using ValueT = std::variant<
            N<Expr>,
            N<Item>,
            N<Stmt>,
            N<Pat>,
            N<Type>,
            N<Block>,
            N<Param>,
            N<AnonConst>,
            N<GenericArg::Lifetime>,
            N<GenericParam>,
            N<CommonField>,
            N<Variant>,
            N<Mod>
        >;

        enum class Kind {
            Expr,
            Item,
            Stmt,
            Pat,
            Type,
            Block,
            Param,
            AnonConst,
            GenericArgLifetime,
            GenericParam,
            CommonField,
            Variant,
            Party,
        };

    private:
        HirNode(Kind kind, ValueT && value) : kind {kind}, value {std::move(value)} {}

    public:
        template<class T>
        static HirNode create(T && node) {
            return HirNode {kindByType<T>(), std::move(node)};
        }

        Kind kind;
        ValueT value;

        template<class T>
        static Kind kindByType() {
            using NT = N<T>;

            // NOTE!!!: If `ValueT` need to contain one type for different nodes kinds,
            //  this method must be deleted and some other way must be used to determine kind by type.
            //  As we cannot know, for example, if `Mod` for `Kind::Crate` and `Mod` for `Kind::Something` is the same.

            if constexpr (std::is_same<NT, Expr>::value) {
                return Kind::Expr;
            }
            if constexpr (std::is_same<NT, Item>::value) {
                return Kind::Item;
            }
            if constexpr (std::is_same<NT, Stmt>::value) {
                return Kind::Stmt;
            }
            if constexpr (std::is_same<NT, Pat>::value) {
                return Kind::Pat;
            }
            if constexpr (std::is_same<NT, Type>::value) {
                return Kind::Type;
            }
            if constexpr (std::is_same<NT, Block>::value) {
                return Kind::Block;
            }
            if constexpr (std::is_same<NT, Param>::value) {
                return Kind::Param;
            }
            if constexpr (std::is_same<NT, AnonConst>::value) {
                return Kind::AnonConst;
            }
            if constexpr (std::is_same<NT, GenericArg::Lifetime>::value) {
                return Kind::GenericArgLifetime;
            }
            if constexpr (std::is_same<NT, GenericParam>::value) {
                return Kind::GenericParam;
            }
            if constexpr (std::is_same<NT, CommonField>::value) {
                return Kind::CommonField;
            }
            if constexpr (std::is_same<NT, Variant>::value) {
                return Kind::Variant;
            }
            if constexpr (std::is_same<NT, Mod>::value) {
                return Kind::Party;
            }

            log::devPanic("Called `HirNode::kindByType` with non-supported `HirNode` type");
        }
    };

    struct OwnerInfo {
        /// The `ChildId` identifier is used instead of `BodyId` as `BodyId` is actually just a `HirId`
        ///  but we know that this bodies belong to current owner, thus no need to store owner `DefId`
        using Bodies = std::map<ChildId, Body>;
        using Nodes = std::map<ChildId, HirNode>;

        OwnerInfo(DefId owner, Bodies && bodies, Nodes && nodes)
            : owner {owner}, bodies {std::move(bodies)}, nodes {std::move(nodes)} {}

        DefId owner;
        Bodies bodies;
        Nodes nodes;

        const HirNode & node(ChildId childId) const {
            return nodes.at(childId);
        }

        const HirNode & ownerNode() const {
            return node(ChildId::ownerChild());
        }

        void addNode(HirId hirId, HirNode && hirNode) {
            if (hirId.owner != owner) {
                log::devPanic("Called `OwnerInfo::addNode` with `HirId` which does not own this `OwnerInfo`");
            }

            nodes.emplace(hirId.id, std::move(hirNode));
        }
    };

    /// The root node of the party (package)
    struct Party {
        using Owners = std::map<DefId, OwnerInfo>;

        Party(Owners && owners) : owners {std::move(owners)} {}

        Owners owners;

        const Mod & partyMod() const {
            return owners.at(DefId::ROOT_DEF_ID).ownerNode().;
        }
    };
}

#endif // JACY_HIR_NODES_PARTY_H
