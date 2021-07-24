#ifndef JACY_AST_ITEM_FUNC_H
#define JACY_AST_ITEM_FUNC_H

#include "ast/item/Item.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/Generics.h"
#include "ast/fragments/Attr.h"
#include "ast/expr/Block.h"

namespace jc::ast {
    struct FuncParam;
    using func_param_list = std::vector<FuncParam>;

    struct FuncParam : Node {
        FuncParam(
            ident_pr name,
            type_ptr type,
            opt_expr_ptr defaultValue,
            const Span & span
        ) : Node(span),
            name(std::move(name)),
            type(std::move(type)),
            defaultValue(std::move(defaultValue)) {}

        ident_pr name;
        type_ptr type;
        opt_expr_ptr defaultValue;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct FuncSig {
        FuncSig(
            const parser::token_list & modifiers,
            func_param_list params,
            opt_type_ptr returnType
        ) : modifiers(modifiers),
            params(std::move(params)),
            returnType(std::move(returnType)) {}

        parser::token_list modifiers;
        func_param_list params;
        opt_type_ptr returnType;
    };

    struct Func : Item {
        Func(
            FuncSig && sig,
            opt_gen_params generics,
            ident_pr name,
            opt_block_ptr body,
            const Span & span
        ) : Item(span, ItemKind::Func),
            sig(std::move(sig)),
            generics(std::move(generics)),
            name(std::move(name)),
            body(std::move(body)) {}

        FuncSig sig;
        opt_gen_params generics;
        ident_pr name;
        opt_block_ptr body;

        span::Ident getName() const override {
            return name.unwrap();
        }

        opt_node_id getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_FUNC_H
