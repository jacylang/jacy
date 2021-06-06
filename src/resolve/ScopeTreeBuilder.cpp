#include "resolve/ScopeTreeBuilder.h"

namespace jc::resolve {
    void ScopeTreeBuilder::visit(const ast::Mod & mod) {
        enterScope();
        visitItems(mod.items);
        exitScope();
    }

    // Helpers //
    void ScopeTreeBuilder::visitItems(const ast::item_list & items) {
        for (const auto & item : items) {
            std::string name;
            switch (item->kind) {
                case ast::ItemKind::Enum: {
                    name = std::static_pointer_cast<ast::Enum>(item)->name.unwrap()->getValue();
                    break;
                }
                case ast::ItemKind::Func: {
                    name = std::static_pointer_cast<ast::Func>(item)->name.unwrap()->getValue();
                    break;
                }
                case ast::ItemKind::Mod: {
                    name = std::static_pointer_cast<ast::Mod>(item)->name.unwrap()->getValue();
                    break;
                }
                case ast::ItemKind::Struct: {
                    name = std::static_pointer_cast<ast::Struct>(item)->name.unwrap()->getValue();
                    break;
                }
                case ast::ItemKind::Trait: {
                    name = std::static_pointer_cast<ast::Trait>(item)->name.unwrap()->getValue();
                    break;
                }
                case ast::ItemKind::TypeAlias: {
                    name = std::static_pointer_cast<ast::TypeAlias>(item)->name.unwrap()->getValue();
                    break;
                }
                default: continue;
            }
            declare(Namespace::Value, name, item->id);
        }
        visitEach(items);
    }

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

    void ScopeTreeBuilder::enterScope() {
        scope = std::make_unique<Scope>(std::move(scope));
    }

    void ScopeTreeBuilder::exitScope() {
        scope = std::move(scope->parent.unwrap("[ScopeTreeBuilder]: Tried to exit global scope"));
    }
}
