#ifndef JACY_AST_EXPR_EXPR_H
#define JACY_AST_EXPR_EXPR_H

#include "ast/Node.h"
#include "ast/BaseVisitor.h"
#include "data_types/Option.h"

namespace jc::ast {
    /**
     * @brief Base expression class
     */
    struct Expr : Node {
        using Ptr = PR<N<Expr>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        enum class Kind {
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
            Subscript,
            Self,
            Tuple,
            Unit,
            Match,
            While,
        };

        Expr(Span span, Kind kind) : Node {span}, kind {kind} {}

        virtual ~Expr() = default;

        Kind kind;

        bool is(Kind kind) const {
            return this->kind == kind;
        }

        bool isSimple() const {
            return kind == Kind::LiteralConstant
                or kind == Kind::Path;
        }

        template<class T>
        static T * as(const N<Expr> & expr) {
            return static_cast<T*>(expr.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_EXPR_EXPR_H
