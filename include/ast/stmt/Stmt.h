#ifndef JACY_STMT_H
#define JACY_STMT_H

#include <vector>

#include "ast/Node.h"
#include "ast/BaseVisitor.h"

namespace jc::ast {
    struct Stmt;
    using stmt_ptr = std::shared_ptr<Stmt>;
    using stmt_list = std::vector<stmt_ptr>;

    enum class StmtType {
        Assignment,
        Expr,
        ClassDecl,
        DoWhile,
        EnumDecl,
        For,
        FuncDecl,
        ObjectDecl,
        VarDecl,
        While,
        TypeAlias,
    };

    struct Stmt : Node {
        Stmt(const Location & loc, StmtType type) : Node(loc), type(type) {}

        StmtType type;

        bool is(StmtType type) const {
            return this->type == type;
        }

        virtual bool isAssignable() const {
            return false;
        }

        virtual void accept(BaseVisitor & visitor) = 0;
    };
}

#endif // JACY_STMT_H
