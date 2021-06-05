#ifndef JACY_AST_FILE_H
#define JACY_AST_FILE_H

#include "ast/item/Item.h"

namespace jc::ast {
    struct File;
    using file_ptr = std::shared_ptr<File>;
    using file_list = std::vector<file_ptr>;

    struct File : Node {
        File(item_list items, const Span & span) : items(std::move(items)), Node(span) {}

        item_list items;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FILE_H
