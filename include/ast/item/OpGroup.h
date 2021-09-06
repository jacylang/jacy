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
            Ident::PR && name,
            SimplePath::Opt && higherThan,
            SimplePath::Opt && lowerThan,
            OpAssoc assoc,
            const Span & span
        ) : Item{span, ItemKind::OpGroup},
            name{std::move(name)},
            higherThan{std::move(higherThan)},
            lowerThan{std::move(lowerThan)},
            assoc{assoc} {}

        Ident::PR name;
        SimplePath::Opt higherThan;
        SimplePath::Opt lowerThan;
        OpAssoc assoc;

        span::Ident getName() const override {
            return name.unwrap();
        }

        NodeId::Opt getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_OPGROUP_H
