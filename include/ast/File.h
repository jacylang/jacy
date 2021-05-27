#ifndef JACY_AST_FILE_H
#define JACY_AST_FILE_H

#include "ast/item/Item.h"

namespace jc::ast {
    struct File;
    using file_ptr = std::shared_ptr<File>;
    using file_list = std::vector<file_ptr>;

    struct File {
        explicit File(item_list items) : items(std::move(items)) {}

        item_list items;
    };
}

#endif // JACY_AST_FILE_H
