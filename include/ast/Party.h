#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include <utility>

#include "ast/nodes.h"

namespace jc::ast {
    class Party {
    public:
        explicit Party(item_list && items) : items(std::move(items)) {}

        item_list items;
    };
}

#endif // JACY_AST_PARTY_H
