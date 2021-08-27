#ifndef JACY_AST_STMT_STMT_H
#define JACY_AST_STMT_STMT_H

#include <vector>

#include "ast/Node.h"
#include "ast/BaseVisitor.h"

namespace jc::ast {
    struct Stmt;
    using StmtPtr = PR<N<Stmt>>;
    using OptStmtPtr = Option<StmtPtr>;
    using stmt_list = std::vector<StmtPtr>;

    enum class StmtKind {
        Expr,
        For,
        Let,
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
        static T * as(const N<Stmt> & stmt) {
            return static_cast<T*>(stmt.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_STMT_STMT_H
