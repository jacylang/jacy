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

    enum class StmtType {
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
        Stmt(const Span & span, StmtType type) : Node(span), type(type) {}

        StmtType type;

        bool is(StmtType type) const {
            return this->type == type;
        }

        virtual void accept(BaseVisitor & visitor) = 0;
    };

    struct ErrorStmt : Stmt {
        explicit ErrorStmt(const Span & span) : Stmt(span, StmtType::Error) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_STMT_H
