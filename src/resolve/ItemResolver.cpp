#include "resolve/ItemResolver.h"

namespace jc::resolve {
    void ItemResolver::visit(ast::FuncDecl * funcDecl) {
        declareItem(funcDecl->name->getValue(), std::make_shared<Item>(Item::Kind::Func, funcDecl->name->id));
    }

    void ItemResolver::declareItem(const std::string & name, Item::Kind kind, ast::node_id nodeId) {
        const auto & found = rib->items.find(name);
        if (found == rib->items.end()) {
            rib->items.emplace(name, std::make_shared<Item>(kind, nodeId));
        }
        suggestCannotRedeclare(name, Item::kindToString(kind), found->second->kindToString(), found->second->nodeId);
    }
}
