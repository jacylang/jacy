#ifndef JACY_AST_FRAGMENTS_ATTR_H
#define JACY_AST_FRAGMENTS_ATTR_H

#include "ast/Node.h"
#include "ast/fragments/Ident.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Attr : Node {
        using Arg = NamedNode<Expr::Ptr, Ident::OptPR>;
        using List = std::vector<Attr>;

        Attr(Ident::PR name, Arg::List params, Span span)
            : Node {span}, name {std::move(name)}, params {std::move(params)} {}

        Ident::PR name;
        Arg::List params;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_ATTR_H
