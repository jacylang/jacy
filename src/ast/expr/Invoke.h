#ifndef JACY_AST_EXPR_INVOKE_H
#define JACY_AST_EXPR_INVOKE_H

#include "ast/fragments/Arg.h"

namespace jc::ast {
    /// `expr '(' ((Identifier ':')? expr (',' (Identifier ':')? expr)* ',')? ')'`
    struct Invoke : Expr {
        Invoke(Expr::Ptr lhs, Arg::List args, Span span)
            : Expr {span, Expr::Kind::Invoke}, lhs {std::move(lhs)}, args {std::move(args)} {}

        Expr::Ptr lhs;
        Arg::List args;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_INVOKE_H
