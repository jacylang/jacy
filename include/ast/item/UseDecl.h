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

        // Just a path
        UseTree(PR<simple_path_ptr> && path, const Span & span)
            : path(std::move(path)), kind(Kind::Raw), Node(span) {}

        // `*`
        UseTree(dt::Option<PR<simple_path_ptr>> && path, bool _, const Span & span)
            : path(std::move(path)), kind(Kind::All), Node(span) {}

        // `as ...`
        UseTree(PR<simple_path_ptr> && path, id_ptr && as, const Span & span)
            : path(std::move(path)), kind(Kind::Rebind), Node(span) {}

        // `{...}`
        UseTree(dt::Option<PR<simple_path_ptr>> && path, use_tree_list && specifics, const Span & span)
            : path(std::move(path)), body(std::move(specifics)), kind(Kind::Specific), Node(span) {}

        dt::Option<PR<simple_path_ptr>> path;
        std::variant<id_ptr, use_tree_list> body;
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
