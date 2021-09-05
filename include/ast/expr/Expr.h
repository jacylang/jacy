#ifndef JACY_AST_EXPR_EXPR_H
#define JACY_AST_EXPR_EXPR_H

#include "ast/Node.h"
#include "ast/BaseVisitor.h"
#include "data_types/Option.h"

namespace jc::ast {
    enum class ExprKind {
        Assign,
        Block,
        Borrow,
        Break,
        Continue,
        Field,
        For,
        If,
        Infix,
        Invoke,
        Lambda,
        List,
        LiteralConstant,
        Loop,
        Paren,
        Path,
        Postfix,
        Prefix,
        Return,
        Spread,
        Struct,
        Subscript,
        Self,
        Tuple,
        Unit,
        Match,
        While,
    };

    struct Expr : Node {
        using Ptr = PR<N<Expr>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        Expr(const Span & span, ExprKind kind) : Node{span}, kind{kind} {}
        virtual ~Expr() = default;

        ExprKind kind;

        bool is(ExprKind kind) const {
            return this->kind == kind;
        }

        bool isSimple() const {
            return kind == ExprKind::LiteralConstant
                   or kind == ExprKind::Path;
        }

        template<class T>
        static T * as(const N<Expr> & expr) {
            return static_cast<T*>(expr.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_EXPR_EXPR_H
