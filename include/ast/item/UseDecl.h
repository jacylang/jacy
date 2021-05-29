#ifndef JACY_AST_ITEM_USEDECL_H
#define JACY_AST_ITEM_USEDECL_H

#include <variant>

#include "ast/item/Item.h"
#include "data_types/Option.h"

namespace jc::ast {
    struct UseDeclPathSeg;
    struct UseTree;
    using use_decl_path_seg_ptr = std::shared_ptr<UseDeclPathSeg>;
    using use_decl_path_seg_list = std::vector<use_decl_path_seg_ptr>;
    using use_tree_ptr = std::shared_ptr<UseTree>;
    using use_tree_list = std::vector<use_tree_ptr>;

    struct UseDeclPathSeg : Node {
        enum class Kind {
            Super,
            Self,
            Party,
            Ident,
        } kind;

        UseDeclPathSeg(id_ptr && ident, const Span & span) : ident(ident), kind(Kind::Ident), Node(span) {}
        UseDeclPathSeg(Kind kind, const Span & span) : kind(kind), Node(span) {}

        dt::Option<id_ptr> ident;
    };

    struct UseDeclPath : Node {
        UseDeclPath(use_decl_path_seg_list && segments, const Span & span)
            : segments(std::move(segments)), Node(span) {}

        use_decl_path_seg_list segments;
    };

    struct UseTree : Node {
        enum class Kind {
            All,
            Specific,
            Rebind,
        } kind;

        // `*` case
        UseTree(const Span & span) : kind(Kind::All), Node(span) {}

        UseTree(id_ptr && as, const Span & span) : kind(Kind::Rebind), Node(span) {}

        UseTree(use_tree_list && specifics, const Span & span) : kind(Kind::Specific), Node(span) {}

        std::variant<id_ptr, use_tree_ptr> body;
    };

    struct UseDecl : Item {
        UseDecl(
            attr_list && attributes,
            std::shared_ptr<UseDeclPath> && path,
            const Span & span
        ) : path(std::move(path)),
            Item(span, std::move(attributes), ItemKind::Use) {}

        std::shared_ptr<UseDeclPath> path;
        use_tree_ptr useTree;
    };
}

#endif // JACY_AST_ITEM_USEDECL_H
