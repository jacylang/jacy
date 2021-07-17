#ifndef JACY_HIR_NODES_STMT_H
#define JACY_HIR_NODES_STMT_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    enum class StmtKind {

    };

    struct Stmt : HirNode {
        Stmt(StmtKind kind, const HirId & hirId, const Span & span) : HirNode(hirId, span), kind(kind) {}

        StmtKind kind;
    };
}

#endif // JACY_HIR_NODES_STMT_H
