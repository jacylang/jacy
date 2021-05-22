#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<rib_stack> NameResolver::resolve(sess::sess_ptr sess, const ast::item_list & tree) {
        typeResolver.setSession(sess);
        itemResolver.setSession(sess);

        for (const auto & item : tree) {
            item->accept(*this);
        }

        sugg::sugg_list suggestions;
        auto typeSuggs = typeResolver.extractSuggestions();
        auto itemSuggs = itemResolver.extractSuggestions();
        suggestions.insert(suggestions.end(), std::make_move_iterator(typeSuggs.begin()), std::make_move_iterator(typeSuggs.end()));
        suggestions.insert(suggestions.end(), std::make_move_iterator(itemSuggs.begin()), std::make_move_iterator(itemSuggs.end()));

        return {ribs, std::move(suggestions)};
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
