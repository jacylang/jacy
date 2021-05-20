#ifndef JACY_EXPR_H
#define JACY_EXPR_H

#include "ast/Node.h"
#include "ast/BaseVisitor.h"
#include "data_types/Option.h"

namespace jc::ast {
    struct Expr;
    using expr_ptr = std::shared_ptr<Expr>;
    using opt_expr_ptr = dt::Option<expr_ptr>;
    using expr_list = std::vector<expr_ptr>;

    enum class ExprType {
        Assign,
        Borrow,
        Break,
        Continue,
        Deref,
        Id,
        If,
        Infix,
        Invoke,
        Lambda,
        List,
        LiteralConstant,
        Loop,
        MemberAccess,
        Paren,
        Path,
        Prefix,
        Quest,
        Return,
        Spread,
        Subscript,
        Super,
        This,
        Tuple,
        Unit,
        When,

        Error,
    };

    struct Expr : Node {
        Expr(const Location & loc, ExprType type) : Node(loc), type(type) {}

        ExprType type;

        bool is(ExprType type) const {
            return this->type == type;
        }

        bool isSimple() const {
            return type == ExprType::LiteralConstant
                or type == ExprType::Id;
        }

        template<class T>
        static std::shared_ptr<T> as(expr_ptr expr) {
            return std::static_pointer_cast<T>(expr);
        }

        virtual void accept(BaseVisitor & visitor) = 0;
    };

    struct ErrorExpr : Expr {
        explicit ErrorExpr(const Location & loc) : Expr(loc, ExprType::Error) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_EXPR_H
