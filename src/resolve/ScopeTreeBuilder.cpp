#include "resolve/ScopeTreeBuilder.h"

namespace jc::resolve {
    void ScopeTreeBuilder::visit(const ast::Mod & mod) {
        enterScope();
        exitScope();
    }

    void ScopeTreeBuilder::declare(Namespace ns, const std::string & name, node_id nodeId) {
        auto & map = scope->getNS(ns);
        if (utils::map::has(map, name)) {
            // TODO: Suggestions
            log.error(name + "has been already declared in this scope");
            return;
        }
        map[name] = nodeId;
    }

    void ScopeTreeBuilder::enterScope() {
        scope = std::make_unique<Scope>(std::move(scope));
    }

    void ScopeTreeBuilder::exitScope() {
        scope = std::move(scope->parent.unwrap("[ScopeTreeBuilder]: Tried to exit global scope"));
    }
}
