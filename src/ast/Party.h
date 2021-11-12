#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include <utility>

#include "ast/nodes.h"

namespace jc::ast {
    class Party {
    public:
        explicit Party(Item::List && items) : items{std::move(items)} {}

        Item::List items;
    };
}

#endif // JACY_AST_PARTY_H
