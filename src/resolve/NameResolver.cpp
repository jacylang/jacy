#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<rib_stack> NameResolver::resolve(sess::sess_ptr sess, const ast::item_list & tree) {
        typeResolver.setSession(sess);
        itemResolver.setSession(sess);

        for (const auto & item : tree) {
            item->accept(*this);
        }

        return {ribs, std::move(moveConcat(
            typeResolver.extractSuggestions(),
            itemResolver.extractSuggestions()
        ))};
    }

    // Statements //
    void NameResolver::visit(ast::ExprStmt * exprStmt) {
        exprStmt->expr->accept(*this);
    }

    void NameResolver::visit(ast::FuncDecl * funcDecl) {

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
        auto newRib = std::make_shared<Rib>();
        ribs.push(newRib);
        ribIndex = ribs.size() - 1;

        typeResolver.acceptRib(newRib);
    }

    void NameResolver::exitRib() {
        if (ribIndex == 0) {
            Logger::devPanic("NameResolver: Tried to exit top-level rib");
        }
        ribIndex--;
    }

}
