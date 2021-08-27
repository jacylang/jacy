#ifndef JACY_AST_ITEM_FUNC_H
#define JACY_AST_ITEM_FUNC_H

#include "ast/item/Item.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/Generics.h"
#include "ast/fragments/Attr.h"
#include "ast/expr/Block.h"
#include "ast/fragments/func_fragments.h"

namespace jc::ast {
    struct Func : Item {
        Func(
            FuncSig && sig,
            GenericParam::OptList generics,
            Ident::PR name,
            Option<Body> && body,
            const Span & span
        ) : Item{span, ItemKind::Func},
            sig{std::move(sig)},
            generics{std::move(generics)},
            name{std::move(name)},
            body{std::move(body)} {}

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
