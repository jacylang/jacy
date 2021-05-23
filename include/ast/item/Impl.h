#ifndef JACY_AST_STMT_IMPL_H
#define JACY_AST_STMT_IMPL_H

#include "ast/item/Item.h"
#include "ast/fragments/TypeParams.h"

namespace jc::ast {
    struct Impl : Item {
        Impl(
            attr_list attributes,
            opt_type_params typeParams,
            type_path_ptr traitTypePath,
            type_ptr forType,
            item_list members,
            const Span & span
        ) : typeParams(std::move(typeParams)),
            traitTypePath(std::move(traitTypePath)),
            forType(std::move(forType)),
            members(std::move(members)),
            Item(span, std::move(attributes), ItemKind::Impl) {}

        opt_type_params typeParams;
        type_path_ptr traitTypePath;
        type_ptr forType;
        item_list members;


        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_IMPL_H
