#ifndef JACY_STMT_H
#define JACY_STMT_H

#include <vector>

#include "ast/Node.h"
#include "ast/BaseVisitor.h"

namespace jc::ast {
    struct Stmt;
    using stmt_ptr = std::shared_ptr<Stmt>;
    using opt_stmt_ptr = dt::Option<stmt_ptr>;
    using stmt_list = std::vector<stmt_ptr>;

    enum class StmtKind {
        Expr,
        Enum,
        For,
        Func,
        Impl,
        Item,
        Struct,
        TypeAlias,
        Trait,
        VarDecl,
        While,

        Error,
    };

    struct Stmt : Node {
        Stmt(const Span & span, StmtKind kind) : Node(span), kind(kind) {}

        StmtKind kind;

        bool is(StmtKind kind) const {
            return this->kind == kind;
        }

        virtual void accept(BaseVisitor & visitor) = 0;
    };

    struct ErrorStmt : Stmt {
        explicit ErrorStmt(const Span & span) : Stmt(span, StmtKind::Error) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_STMT_H
