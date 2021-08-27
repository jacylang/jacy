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
            Ident::PR name,
            type_ptr type,
            Expr::OptPtr defaultValue,
            const Span & span
        ) : Node(span),
            name(std::move(name)),
            type(std::move(type)),
            defaultValue(std::move(defaultValue)) {}

        Ident::PR name;
        type_ptr type;
        Expr::OptPtr defaultValue;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Body {
        Body(bool exprBody, Expr::Ptr && value) : exprBody(exprBody), value(std::move(value)) {}

        bool exprBody;
        Expr::Ptr value;
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
            GenericParam::OptList generics,
            Ident::PR name,
            Option<Body> && body,
            const Span & span
        ) : Item(span, ItemKind::Func),
            sig(std::move(sig)),
            generics(std::move(generics)),
            name(std::move(name)),
            body(std::move(body)) {}

        FuncSig sig;
        GenericParam::OptList generics;
        Ident::PR name;
        Option<Body> body;

        span::Ident getName() const override {
            return name.unwrap();
        }

        OptNodeId getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_FUNC_H
