#ifndef JACY_SRC_AST_FRAGMENTS_ANONCONST_H
#define JACY_SRC_AST_FRAGMENTS_ANONCONST_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct AnonConst {
        using Opt = Option<AnonConst>;

        // TODO: Think how to get rid of useless `Span` from `Node` (`AnonConst` need to be child of `Node`)

        AnonConst(Expr::Ptr && expr) : expr {std::move(expr)} {}

        Expr::Ptr expr;
        NodeId id;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_SRC_AST_FRAGMENTS_ANONCONST_H
