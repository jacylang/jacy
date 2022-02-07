#ifndef JACY_SRC_AST_FRAGMENTS_ANONCONST_H
#define JACY_SRC_AST_FRAGMENTS_ANONCONST_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct AnonConst {
        using Opt = Option<AnonConst>;

        AnonConst(NodeId nodeId, Expr::Ptr && expr) : nodeId {nodeId}, expr {std::move(expr)} {}

        NodeId nodeId;
        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_SRC_AST_FRAGMENTS_ANONCONST_H
