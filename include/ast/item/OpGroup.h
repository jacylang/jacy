#ifndef JACY_AST_ITEM_OPGROUP_H
#define JACY_AST_ITEM_OPGROUP_H

#include "ast/item/Item.h"

namespace jc::ast {
    struct OpGroup : Item {
        OpGroup(const Span & span) {}
    };
}

#endif // JACY_AST_ITEM_OPGROUP_H
