#ifndef JACY_AST_ITEM_INIT_H
#define JACY_AST_ITEM_INIT_H

#include "ast/item/Item.h"
#include "ast/fragments/func_fragments.h"

namespace jc::ast {
    struct Init : Item {
        Init(
            FuncSig && sig,
            Ident::PR name,
            Option<Body> && body,
            const Span & span
        ) : Item{span, ItemKind::Init},
            sig{std::move(sig)},
            name{std::move(name)},
            body{std::move(body)} {}

        FuncSig sig;
        Ident::PR name;
        Option<Body> body;
    };
}

#endif // JACY_AST_ITEM_INIT_H
