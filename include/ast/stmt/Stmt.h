#ifndef JACY_AST_STMT_STMT_H
#define JACY_AST_STMT_STMT_H

#include <vector>

#include "ast/Node.h"
#include "ast/BaseVisitor.h"

namespace jc::ast {
    struct Stmt;
    using stmt_ptr = PR<N<Stmt>>;
    using opt_stmt_ptr = Option<stmt_ptr>;
    using stmt_list = std::vector<stmt_ptr>;

    enum class StmtKind {
        Expr,
        For,
        Var,
        While,
        Item,
    };

    struct Stmt : Node {
        Stmt(const Span & span, StmtKind kind) : Node(span), kind(kind) {}

        StmtKind kind;

        bool is(StmtKind kind) const {
            return this->kind == kind;
        }

        template<class T>
        static T as(Stmt * stmt) {
            return *N<T>(static_cast<T*>(stmt));
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_STMT_STMT_H
