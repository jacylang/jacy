#ifndef JACY_AST_STMT_STMT_H
#define JACY_AST_STMT_STMT_H

#include <vector>

#include "ast/Node.h"
#include "ast/BaseVisitor.h"

namespace jc::ast {
    struct Stmt : Node {
        using Ptr = PR<N<Stmt>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        enum class Kind {
            Expr,
            Let,
            Item,
        };

        Stmt(Span span, Kind kind) : Node {span}, kind {kind} {}

        Kind kind;

        bool is(Kind kind) const {
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
