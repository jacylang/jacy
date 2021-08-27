#ifndef JACY_AST_EXPR_EXPR_H
#define JACY_AST_EXPR_EXPR_H

#include "ast/Node.h"
#include "ast/BaseVisitor.h"
#include "data_types/Option.h"

namespace jc::ast {
    struct Expr;
    using ExprPtr = PR<N<Expr>>;
    using OptExprPtr = Option<ExprPtr>;
    using expr_list = std::vector<ExprPtr>;

    enum class ExprKind {
        Assign,
        Block,
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
        Struct,
        Subscript,
        Super,
        This,
        Tuple,
        Unit,
        Match,
    };

    struct Expr : Node {
        Expr(const Span & span, ExprKind kind) : Node(span), kind(kind) {}
        virtual ~Expr() = default;

        ExprKind kind;

        bool is(ExprKind kind) const {
            return this->kind == kind;
        }

        bool isSimple() const {
            return kind == ExprKind::LiteralConstant
                   or kind == ExprKind::Id;
        }

        template<class T>
        static T * as(const N<Expr> & expr) {
            return static_cast<T*>(expr.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_EXPR_EXPR_H
