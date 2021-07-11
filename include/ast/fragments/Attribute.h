#ifndef JACY_AST_FRAGMENTS_ATTRIBUTE_H
#define JACY_AST_FRAGMENTS_ATTRIBUTE_H

#include "ast/Node.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/Arg.h"

namespace jc::ast {
    struct Attribute;
    using attr_ptr = N<Attribute>;
    using attr_list = std::vector<attr_ptr>;

    struct Attribute : Node {
        Attribute(id_ptr name, arg_list params, const Span & span)
            : Node(span), name(std::move(name)), params(std::move(params)) {}

        id_ptr name;
        arg_list params;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_ATTRIBUTE_H
