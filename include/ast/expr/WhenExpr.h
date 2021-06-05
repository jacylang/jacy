#ifndef JACY_AST_EXPR_WHENEXPR_H
#define JACY_AST_EXPR_WHENEXPR_H

#include "ast/expr/Expr.h"
#include "Block.h"

namespace jc::ast {
    struct WhenEntry;
    using when_entry_ptr = std::shared_ptr<WhenEntry>;
    using when_entry_list = std::vector<when_entry_ptr>;

    struct WhenEntry : Node {
        WhenEntry(
            expr_list conditions,
            block_ptr body,
            const Span & span
        ) : conditions(std::move(conditions)),
            body(std::move(body)),
            Node(span) {}

        // TODO: Complex prefix conditions like `in lhs`
        expr_list conditions;
        block_ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct WhenExpr : Expr {
        WhenExpr(
            expr_ptr subject,
            when_entry_list entries,
            const Span & span
        ) : subject(std::move(subject)),
            entries(std::move(entries)),
            Expr(span, ExprKind::When) {}

        expr_ptr subject;
        when_entry_list entries;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif //JACY_AST_EXPR_WHENEXPR_H
