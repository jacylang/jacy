#ifndef JACY_EXPR_H
#define JACY_EXPR_H

#include "tree/Node.h"

namespace jc::tree {
    struct Expr;
    using expr_ptr = std::shared_ptr<Expr>;
    using expr_list = std::vector<expr_ptr>;

    enum class ExprType {
        Break,
        Continue,
        Id,
        If,
        Infix,
        Invoke,
        List,
        LiteralConstant,
        Paren,
        Postfix,
        Prefix,
        Return,
        Spread,
        Subscript,
        Super,
        This,
        Throw,
        TryCatch,
        Tuple,
        When,
    };

    struct Expr : Node {
        Expr(const Location & loc, ExprType type) : Node(loc), type(type) {}

        ExprType type;

        bool is(ExprType type) const {
            return this->type == type;
        }

        template<class T>
        static std::shared_ptr<T> as(expr_ptr expr) {
            return std::static_pointer_cast<T>(expr);
        }

        virtual bool isAssignable() const {
            return false;
        }
    };
}

#endif // JACY_EXPR_H
