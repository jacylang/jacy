#ifndef JACY_AST_ITEM_USEDECL_H
#define JACY_AST_ITEM_USEDECL_H

#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/Path.h"

namespace jc::ast {
    struct UseTree;
    using use_tree_ptr = std::shared_ptr<UseTree>;
    using use_tree_list = std::vector<use_tree_ptr>;

    struct UseTree : Node {
        enum class Kind {
            Raw,
            All,
            Specific,
            Rebind,
        } kind;

        UseTree(Kind kind, const Span & span) : Node(span) {}
    };

    struct UseTreeRaw : UseTree {
        UseTreeRaw(simple_path_ptr && path, const Span & span)
            : path(std::move(path)), UseTree(Kind::Raw, span) {}

        simple_path_ptr path;
    };

    struct UseTreeSpecific : UseTree {
        UseTreeSpecific(dt::Option<simple_path_ptr> && path, use_tree_list && specifics, const Span & span)
            : path(std::move(path)), body(std::move(specifics)), UseTree(Kind::Specific, span) {}

        dt::Option<simple_path_ptr> path;
        use_tree_list body;
    };

    struct UseTreeRebind : UseTree {
        UseTreeRebind(simple_path_ptr && path, id_ptr && as, const Span & span)
            : path(std::move(path)), as(std::move(as)), UseTree(Kind::Rebind, span) {}

        simple_path_ptr path;
        id_ptr as;
    };

    struct UseTreeAll : UseTree {
        UseTreeAll(dt::Option<simple_path_ptr> && path, const Span & span)
            : path(std::move(path)), UseTree(Kind::All, span) {}

        dt::Option<simple_path_ptr> path;
    };

    struct UseDecl : Item {
        UseDecl(
            attr_list && attributes,
            use_tree_ptr && useTree,
            const Span & span
        ) : useTree(std::move(useTree)),
            Item(span, std::move(attributes), ItemKind::Use) {}

        use_tree_ptr useTree;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_USEDECL_H
