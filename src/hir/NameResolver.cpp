#include "hir/NameResolver.h"

namespace jc::hir {
    // Statements //
    void NameResolver::visit(ast::ExprStmt * exprStmt) {
        exprStmt->expr->accept(*this);
    }

    void NameResolver::visit(ast::Item * item) {
        item->stmt->accept(*this);
    }

    // Expressions //
    void NameResolver::visit(ast::Block * block) {
        enterRib();
        for (const auto & stmt : block->stmts) {
            stmt->accept(*this);
        }
        exitRib();
    }

    // Ribs //
    void NameResolver::enterRib() {
        ribs.push(std::make_unique<Rib>());
        ribIndex = ribs.size() - 1;
    }

    void NameResolver::exitRib() {
        if (ribIndex == 0) {
            Logger::devPanic("NameResolver: Tried to exit top-level rib");
        }
        ribIndex--;
    }

    // Resolution //
}
