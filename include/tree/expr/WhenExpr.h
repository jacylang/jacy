#ifndef JACY_WHENEXPR_H
#define JACY_WHENEXPR_H

#include "tree/expr/Expr.h"
#include "tree/fragments/Block.h"

namespace jc::tree {
    struct WhenEntry;
    using when_entry_ptr = std::shared_ptr<WhenEntry>;
    using when_entry_list = std::vector<when_entry_ptr>;

    struct WhenEntry : Node {
        WhenEntry(
            expr_list conditions,
            block_ptr body,
            expr_ptr oneLineBody,
            const Location & loc
        ) : conditions(conditions),
            body(body),
            oneLineBody(oneLineBody),
            Node(loc) {}

        // TODO: Complex prefix conditions like `in expr`
        expr_list conditions;
        block_ptr body;
        expr_ptr oneLineBody;
    };

    struct WhenExpr : Expr {
        WhenExpr(
            expr_ptr subject,
            when_entry_list entries,
            const Location & loc
        ) : subject(subject),
            entries(entries),
            Expr(loc, ExprType::When) {}

        expr_ptr subject;
        when_entry_list entries;
    };
}

#endif //JACY_WHENEXPR_H
