#ifndef JACY_AST_STMT_LETSTMT_H
#define JACY_AST_STMT_LETSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/Type.h"
#include "ast/fragments/Pattern.h"

namespace jc::ast {
    struct LetStmt : Stmt {
        LetStmt(
            pat_ptr && pat,
            opt_type_ptr && type,
            opt_expr_ptr && assignExpr,
            const Span & span
        ) : Stmt(span, StmtKind::Var),
            pat(std::move(pat)),
            type(std::move(type)),
            assignExpr(std::move(assignExpr)) {}

        pat_ptr pat;
        opt_type_ptr type;
        opt_expr_ptr assignExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_LETSTMT_H
