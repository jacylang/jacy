#ifndef JACY_AST_FILE_H
#define JACY_AST_FILE_H

#include "ast/item/Item.h"
#include "ast/DirTreePrinter.h"

namespace jc::ast {
    struct File;
    using file_ptr = std::shared_ptr<File>;

    struct File : Node {
        File(
            span::file_id_t fileId,
            item_list items
        ) : Node(Span{fileId}),
            fileId(fileId),
            items(std::move(items)) {}

        span::file_id_t fileId;
        item_list items;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }

        void accept(DirTreePrinter & visitor) const {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FILE_H
