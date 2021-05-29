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
            All,
            Specific,
            Rebind,
        } kind;

        // `*`
        explicit UseTree(const Span & span) : kind(Kind::All), Node(span) {}

        // `as ...`
        UseTree(id_ptr && as, const Span & span) : kind(Kind::Rebind), Node(span) {}

        // `{...}`
        UseTree(use_tree_list && specifics, const Span & span) : kind(Kind::Specific), Node(span) {}

        std::variant<id_ptr, use_tree_ptr> body;
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
