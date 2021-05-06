#ifndef JACY_LITERALCONSTANT_H
#define JACY_LITERALCONSTANT_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct LiteralConstant;
    using literal_ptr = std::shared_ptr<LiteralConstant>;

    struct LiteralConstant : Expr {
        LiteralConstant(const parser::Token & token, const Location & loc)
            : token(token), Expr(token.loc, ExprType::LiteralConstant) {}

        parser::Token token;
    };
}

#endif // JACY_LITERALCONSTANT_H
