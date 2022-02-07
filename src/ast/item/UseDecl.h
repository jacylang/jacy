#ifndef JACY_AST_ITEM_USEDECL_H
#define JACY_AST_ITEM_USEDECL_H

#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/SimplePath.h"

namespace jc::ast {
    struct UseTree : Node {
        using PR = PR<UseTree>;
        using List = std::vector<PR>;
        using ValueT = std::variant<std::monostate, Ident::PR, UseTree::List>;

        enum class Kind {
            Raw,
            All,
            Specific,
            Rebind,
        };

        UseTree(SimplePath::Opt && path, span::Span span)
            : Node {span}, kind {Kind::Raw}, path {std::move(path)}, val {std::monostate {}} {}

        UseTree(SimplePath::Opt && path, bool, span::Span span)
            : Node {span}, kind {Kind::All}, path {std::move(path)}, val {std::monostate {}} {}

        UseTree(SimplePath::Opt && path, UseTree::List && specifics, span::Span span)
            : Node {span}, kind {Kind::Specific}, path {std::move(path)}, val {std::move(specifics)} {}

        UseTree(SimplePath::Opt && path, Ident::PR && rebinding, span::Span span)
            : Node {span}, kind {Kind::Rebind}, path {std::move(path)}, val {std::move(rebinding)} {}

        Kind kind;
        SimplePath::Opt path;
        ValueT val;

        const auto & expectPath() const {
            return path.unwrap();
        }

        const auto & expectRebinding() const {
            return std::get<Ident::PR>(val).unwrap();
        }

        const auto & expectSpecifics() const {
            return std::get<UseTree::List>(val);
        }

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct UseDecl : Item {
        UseDecl(
            UseTree::PR && useTree,
            Span span
        ) : Item {span, Item::Kind::Use},
            useTree {std::move(useTree)} {}

        UseTree::PR useTree;

        span::Ident getName() const override {
            return Ident::empty();
        }

        NodeId::Opt getNameNodeId() const override {
            return None;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_USEDECL_H
