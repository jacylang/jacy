#include "resolve/ScopeTreeBuilder.h"

namespace jc::resolve {
    void ScopeTreeBuilder::visit(const ast::Mod & mod) {
        enterScope(mod.name.unwrap()->getValue());
        visitEach(mod.items);
        exitScope();
    }

    void ScopeTreeBuilder::visit(const ast::Trait & trait) {
        enterScope(trait.name.unwrap()->getValue());
        visitEach(trait.members);
        exitScope();
    }



//    void ScopeTreeBuilder::visit(const ast::Struct & _struct) {
//        enterScope(_struct.name.unwrap()->getValue());
//        // TODO: Update for associated items in the future
//        exitScope();
//    }

    // Scopes //
    void ScopeTreeBuilder::declare(Namespace ns, const std::string & name, node_id nodeId) {
        auto & map = scope->getNS(ns);
        if (utils::map::has(map, name)) {
            // TODO: Suggestions
            log.error(name + "has been already declared in this scope");
            return;
        }
        map[name] = nodeId;
    }

    void ScopeTreeBuilder::enterScope(const dt::Option<std::string> & name) {
        auto child = new Scope(scope);
        if (name) {
            // TODO: Check for redeclaration
            scope->children.emplace(name.unwrap(), child);
        }
        scope = child;
    }

    void ScopeTreeBuilder::exitScope() {
        scope = std::move(scope->parent.unwrap("[ScopeTreeBuilder]: Tried to exit global scope"));
    }
}
