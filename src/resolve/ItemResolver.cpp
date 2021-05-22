#include "resolve/ItemResolver.h"

namespace jc::resolve {
    void ItemResolver::visit(ast::FuncDecl * funcDecl) {
        declareItem(funcDecl->name->getValue(), Item::Kind::Func, funcDecl->name->id);
    }

    void ItemResolver::declareItem(const std::string & name, Item::Kind kind, ast::node_id nodeId) {
        const auto & found = rib->items.find(name);
        if (found == rib->items.end()) {
            rib->items.emplace(name, std::make_shared<Item>(kind, nodeId));
            return;
        }
        suggestCannotRedeclare(name, Item::kindStr(kind), found->second->kindStr(), nodeId, found->second->nodeId);
    }
}
