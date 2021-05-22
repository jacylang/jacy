#include "resolve/ItemResolver.h"

namespace jc::resolve {
    void ItemResolver::visit(ast::FuncDecl * funcDecl) {
        declareItem(funcDecl->name->getValue(), std::make_shared<Item>(Item::Kind::Func, funcDecl->name->id));
    }

    void ItemResolver::declareItem(const std::string & name, item_ptr item) {
        if (rib->items.find(name) == rib->items.end()) {
            rib->items.emplace(name, item);
        }

    }
}
