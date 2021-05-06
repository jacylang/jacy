#ifndef JACY_IDENTIFIER_H
#define JACY_IDENTIFIER_H

#include <vector>

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct Identifier;
    using id_ptr = std::shared_ptr<Identifier>;
    using IdList = std::vector<id_ptr>;

    struct Identifier : Expr {
        Identifier(const Location & loc) : Expr(loc, ExprType::Id) {}

        parser::Token token;
    };
}

#endif // JACY_IDENTIFIER_H
