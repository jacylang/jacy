#include "resolve/ItemResolver.h"

namespace jc::resolve {
    void ItemResolver::visit(ast::FuncDecl * funcDecl) {
        declareItem(funcDecl->name->getValue(), std::make_shared<Item>(Item::Kind::Func, funcDecl->name->id));
    }

    void ItemResolver::declareItem(const std::string & name, item_ptr item) {
        const auto & found = rib->items.find(name);
        if (found == rib->items.end()) {
            rib->items.emplace(name, item);
        }
        suggestCannotRedeclare(name, item->kindToString(), found->second->kindToString(), found->second->nodeId);
    }
}
