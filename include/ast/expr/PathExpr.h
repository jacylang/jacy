#ifndef JACY_AST_PATHEXPR_H
#define JACY_AST_PATHEXPR_H

#include "ast/expr/Expr.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/TypeParams.h"

namespace jc::ast {
    struct PathExprSeg : Node {
        PathExprSeg() {}

        id_ptr id;
        type_param_list typeParams;
    };

    struct PathExpr : Expr {
        PathExpr(bool global, )
    };
}

#endif // JACY_AST_PATHEXPR_H
