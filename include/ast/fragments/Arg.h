#ifndef JACY_AST_FRAGMENTS_ARG_H
#define JACY_AST_FRAGMENTS_ARG_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Ident.h"

namespace jc::ast {
    struct Arg;
    using arg_list = std::vector<Arg>;

    struct Arg : Node {
        Arg(opt_ident && name, expr_ptr && value, const Span & span)
            : Node(span), name(std::move(name)), value(std::move(value)) {}

        opt_ident name;
        expr_ptr value;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_ARG_H
