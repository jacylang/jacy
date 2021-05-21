#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include "ast/nodes.h"

namespace jc::ast {
    class Party;
    using party_ptr = std::shared_ptr<Party>;

    class Party {
    public:
        Party(item_list && items) : items(std::move(items)) {}

        const item_list & getItems() const {
            return items;
        }

    private:
        item_list items;
    };
}

#endif // JACY_AST_PARTY_H
