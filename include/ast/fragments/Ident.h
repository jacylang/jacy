#ifndef JACY_AST_FRAGMENTS_IDENT_H
#define JACY_AST_FRAGMENTS_IDENT_H

#include <vector>

#include "ast/Node.h"
#include "span/Ident.h"

namespace jc::ast {
    using ident_pr = PR<span::Ident>;
    using opt_ident = Option<ident_pr>;

    struct Ident : span::Ident {
        Ident(const parser::Token & token) : span::Ident(token) {}

        void accept(BaseVisitor & visitor) const {
            visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_IDENT_H
