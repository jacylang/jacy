#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<rib_stack> NameResolver::resolve(sess::sess_ptr sess, const ast::item_list & tree) {
        typeResolver = std::make_unique<TypeResolver>(sess);
        itemResolver = std::make_unique<ItemResolver>(sess);

        enterRib();

        for (const auto & item : tree) {
            item->accept(*this);
        }

        return {ribs, moveConcat(
            typeResolver->extractSuggestions(),
            itemResolver->extractSuggestions()
        )};
    }

    // Statements //
    void NameResolver::visit(ast::ExprStmt * exprStmt) {
        exprStmt->expr->accept(*this);
    }

    void NameResolver::visit(ast::FuncDecl * funcDecl) {
        itemResolver->visit(funcDecl);

        enterRib(); // Enter type rib
        typeResolver->visit(funcDecl);

        if (funcDecl->oneLineBody) {
            funcDecl->oneLineBody.unwrap()->accept(*this);
        } else {
            visit(funcDecl->body.unwrap().get());
        }

        exitRib();
    }

    void NameResolver::visit(ast::Impl * impl) {
        itemResolver->visit(impl);
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

        typeResolver->acceptRib(newRib);
        itemResolver->acceptRib(newRib);
    }

    void NameResolver::exitRib() {
        if (ribIndex == 0) {
            Logger::devPanic("NameResolver: Tried to exit top-level rib");
        }
        ribIndex--;
    }

}
