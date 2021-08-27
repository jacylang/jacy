#ifndef JACY_AST_ITEM_USEDECL_H
#define JACY_AST_ITEM_USEDECL_H

#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/SimplePath.h"

namespace jc::ast {
    struct UseTree : Node {
        using Ptr = PR<N<UseTree>>;
        using List = std::vector<Ptr>;

        enum class Kind {
            Raw,
            All,
            Specific,
            Rebind,
        } kind;

        UseTree(Kind kind, const Span & span) : Node{span}, kind{kind} {}

        virtual void accept(BaseVisitor & visitor) const override = 0;
    };

    struct UseTreeRaw : UseTree {
        UseTreeRaw(SimplePath && path, const Span & span)
            : UseTree(Kind::Raw, span), path(std::move(path)) {}

        SimplePath path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseTreeSpecific : UseTree {
        UseTreeSpecific(Option<SimplePath> && path, UseTree::List && specifics, const Span & span)
            : UseTree(Kind::Specific, span), path(std::move(path)), specifics(std::move(specifics)) {}

        Option<SimplePath> path;
        UseTree::List specifics;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseTreeRebind : UseTree {
        UseTreeRebind(SimplePath && path, Ident::PR && as, const Span & span)
            : UseTree(Kind::Rebind, span), path(std::move(path)), as(std::move(as)) {}

        SimplePath path;
        Ident::PR as;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseTreeAll : UseTree {
        UseTreeAll(Option<SimplePath> && path, const Span & span)
            : UseTree(Kind::All, span), path(std::move(path)) {}

        Option<SimplePath> path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseDecl : Item {
        UseDecl(
            UseTree::Ptr && useTree,
            const Span & span
        ) : Item{span, ItemKind::Use},
            useTree(std::move(useTree)) {}

        UseTree::Ptr useTree;

        span::Ident getName() const override {
            return Ident::empty();
        }

        OptNodeId getNameNodeId() const override {
            return None;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_USEDECL_H
