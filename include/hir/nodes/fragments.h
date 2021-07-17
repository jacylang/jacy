#ifndef JACY_HIR_NODES_FRAGMENTS_H
#define JACY_HIR_NODES_FRAGMENTS_H

#include "hir/nodes/HirNode.h"
#include "span/Ident.h"

namespace jc::hir {
    struct Arg;
    using arg_list = std::vector<Arg>;
    struct Expr;
    struct Stmt;
    using expr_ptr = N<Expr>;
    using stmt_ptr = N<Stmt>;
    using stmt_list = N<stmt_ptr>;

    struct Arg : HirNode {
        Arg(const span::Ident & ident, expr_ptr && value, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), ident(ident), value(std::move(value)) {}

        span::Ident ident;
        expr_ptr value;
    };

    struct Block : HirNode {
        Block(stmt_list && stmts, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), stmts(std::move(stmts)) {}

        stmt_list stmts;
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
