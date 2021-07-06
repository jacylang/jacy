#ifndef JACY_AST_ITEM_FUNC_H
#define JACY_AST_ITEM_FUNC_H

#include "ast/item/Item.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/Generics.h"
#include "ast/fragments/Attribute.h"
#include "ast/expr/Block.h"

namespace jc::ast {
    struct FuncParam;
    using func_param_ptr = N<FuncParam>;
    using func_param_list = std::vector<func_param_ptr>;

    struct FuncParam : Node {
        FuncParam(
            id_ptr name,
            type_ptr type,
            opt_expr_ptr defaultValue,
            const Span & span
        ) : Node(span),
            name(std::move(name)),
            type(std::move(type)),
            defaultValue(std::move(defaultValue)) {}

        id_ptr name;
        type_ptr type;
        opt_expr_ptr defaultValue;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Func : Item {
        Func(
            parser::token_list modifiers,
            opt_gen_params generics,
            id_ptr name,
            func_param_list params,
            opt_type_ptr returnType,
            opt_block_ptr body,
            const Span & span
        ) : Item(span, ItemKind::Func),
            modifiers(std::move(modifiers)),
            generics(std::move(generics)),
            name(std::move(name)),
            params(std::move(params)),
            returnType(std::move(returnType)),
            body(std::move(body)) {}

        parser::token_list modifiers;
        opt_gen_params generics;
        id_ptr name;
        func_param_list params;
        opt_type_ptr returnType;
        opt_block_ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_FUNC_H
