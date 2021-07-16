#ifndef JACY_AST_FRAGMENTS_ATTR_H
#define JACY_AST_FRAGMENTS_ATTR_H

#include "ast/Node.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/Arg.h"

namespace jc::ast {
    struct Attr;
    using attr_list = std::vector<Attr>;

    struct Attr : Node {
        Attr(ident_pr name, arg_list params, const Span & span)
            : Node(span), name(std::move(name)), params(std::move(params)) {}

        ident_pr name;
        arg_list params;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_ATTR_H
