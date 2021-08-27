#ifndef JACY_AST_ITEM_IMPL_H
#define JACY_AST_ITEM_IMPL_H

#include "ast/item/Item.h"
#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct Impl : Item {
        Impl(
            GenericParam::OptList && generics,
            PR<type_path_ptr> && traitTypePath,
            opt_type_ptr && forType,
            item_list && members,
            const Span & span
        ) : Item(span, ItemKind::Impl),
            generics(std::move(generics)),
            traitTypePath(std::move(traitTypePath)),
            forType(std::move(forType)),
            members(std::move(members)) {}

        GenericParam::OptList generics;
        PR<type_path_ptr> traitTypePath;
        opt_type_ptr forType;
        item_list members;

        span::Ident getName() const override {
            return Ident::empty();
        }

        OptNodeId getNameNodeId() const override {
            return None;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_IMPL_H
