#ifndef JACY_AST_ITEM_USEDECL_H
#define JACY_AST_ITEM_USEDECL_H

#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/SimplePath.h"

namespace jc::ast {
    struct UseTree;
    using use_tree_ptr = PR<P<UseTree>>;
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
        UseTreeRaw(simple_path_ptr && path, const Span & span)
            : UseTree(Kind::Raw, span), path(std::move(path)) {}

        simple_path_ptr path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseTreeSpecific : UseTree {
        UseTreeSpecific(dt::Option<simple_path_ptr> && path, use_tree_list && specifics, const Span & span)
            : UseTree(Kind::Specific, span), path(std::move(path)), specifics(std::move(specifics)) {}

        dt::Option<simple_path_ptr> path;
        use_tree_list specifics;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseTreeRebind : UseTree {
        UseTreeRebind(simple_path_ptr && path, id_ptr && as, const Span & span)
            : UseTree(Kind::Rebind, span), path(std::move(path)), as(std::move(as)) {}

        simple_path_ptr path;
        id_ptr as;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseTreeAll : UseTree {
        UseTreeAll(dt::Option<simple_path_ptr> && path, const Span & span)
            : UseTree(Kind::All, span), path(std::move(path)) {}

        dt::Option<simple_path_ptr> path;

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

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_USEDECL_H
