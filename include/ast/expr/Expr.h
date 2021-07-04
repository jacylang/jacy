#ifndef JACY_AST_EXPR_EXPR_H
#define JACY_AST_EXPR_EXPR_H

#include "ast/Node.h"
#include "ast/BaseVisitor.h"
#include "data_types/Option.h"

namespace jc::ast {
    struct Expr;
    using pure_expr_ptr = N<Expr>;
    using expr_ptr = PR<pure_expr_ptr>;
    using opt_expr_ptr = dt::Option<expr_ptr>;
    using expr_list = std::vector<expr_ptr>;

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
        When,
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
        static N<T> as(pure_expr_ptr expr) {
            return std::static_pointer_cast<T>(expr);
        }

        template<class T = pure_expr_ptr>
        static expr_ptr pureAsBase(T && expr) {
            return std::static_pointer_cast<Expr>(expr);
        }

        template<typename T = expr_ptr>
        static expr_ptr asBase(T && expr) {
            if (expr.isErr()) {
                return expr.asErr();
            }
            return std::static_pointer_cast<Expr>(expr.asValue());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_EXPR_EXPR_H
