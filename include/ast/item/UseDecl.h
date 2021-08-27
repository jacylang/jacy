#ifndef JACY_AST_ITEM_USEDECL_H
#define JACY_AST_ITEM_USEDECL_H

#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/SimplePath.h"

namespace jc::ast {
    struct UseTree;
    using use_tree_ptr = PR<N<UseTree>>;
    using use_tree_list = std::vector<use_tree_ptr>;

    struct UseTree : Node {
        enum class Kind {
            Raw,
            All,
            Specific,
            Rebind,
        } kind;

        UseTree(Kind kind, const Span & span) : Node(span), kind(kind) {}

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
        UseTreeSpecific(Option<SimplePath> && path, use_tree_list && specifics, const Span & span)
            : UseTree(Kind::Specific, span), path(std::move(path)), specifics(std::move(specifics)) {}

        Option<SimplePath> path;
        use_tree_list specifics;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseTreeRebind : UseTree {
        UseTreeRebind(SimplePath && path, ident_pr && as, const Span & span)
            : UseTree(Kind::Rebind, span), path(std::move(path)), as(std::move(as)) {}

        SimplePath path;
        ident_pr as;

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
            use_tree_ptr && useTree,
            const Span & span
        ) : Item(span, ItemKind::Use),
            useTree(std::move(useTree)) {}

        use_tree_ptr useTree;

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
