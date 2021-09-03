#ifndef JACY_AST_ITEM_OPGROUP_H
#define JACY_AST_ITEM_OPGROUP_H

#include "ast/item/Item.h"
#include "ast/fragments/SimplePath.h"

namespace jc::ast {
    enum class OpAssoc {
        Left,
        Right,
    };

    struct OpGroup : Item {
        OpGroup(
            SimplePath && higherThan,
            SimplePath && lowerThan,
            OpAssoc assoc,
            const Span & span
        ) : Item{span, ItemKind::OpGroup},
            higherThan{std::move(higherThan)},
            lowerThan{std::move(lowerThan)},
            assoc{assoc} {}

        SimplePath higherThan;
        SimplePath lowerThan;
        OpAssoc assoc;
    };
}

#endif // JACY_AST_ITEM_OPGROUP_H
