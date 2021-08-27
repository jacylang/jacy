#ifndef JACY_AST_ITEM_INIT_H
#define JACY_AST_ITEM_INIT_H

#include "ast/item/Item.h"
#include "ast/fragments/func_fragments.h"

namespace jc::ast {
    struct Init : Item {
        Init(
            FuncSig && sig,
            Option<Body> && body,
            const Span & span
        ) : Item{span, ItemKind::Init},
            sig{std::move(sig)},
            body{std::move(body)} {}

        FuncSig sig;
        Option<Body> body;

        span::Ident getName() const override {
            return Ident::empty();
        }

        NodeId::Opt getNameNodeId() const override {
            return None;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_INIT_H
