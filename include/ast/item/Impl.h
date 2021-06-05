#ifndef JACY_AST_ITEM_IMPL_H
#define JACY_AST_ITEM_IMPL_H

#include "ast/item/Item.h"
#include "ast/fragments/TypeParams.h"

namespace jc::ast {
    struct Impl : Item {
        Impl(
            opt_type_params typeParams,
            PR<type_path_ptr> traitTypePath,
            type_ptr forType,
            item_list members,
            const Span & span
        ) : typeParams(std::move(typeParams)),
            traitTypePath(std::move(traitTypePath)),
            forType(std::move(forType)),
            members(std::move(members)),
            Item(span, ItemKind::Impl) {}

        opt_type_params typeParams;
        PR<type_path_ptr> traitTypePath;
        type_ptr forType;
        item_list members;


        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_IMPL_H
