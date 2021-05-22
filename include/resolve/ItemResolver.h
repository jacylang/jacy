#ifndef JACY_RESOLVE_ITEMRESOLVER_H
#define JACY_RESOLVE_ITEMRESOLVER_H

#include "resolve/BaseResolver.h"

namespace jc::resolve {
    class ItemResolver : public BaseResolver {
    public:
        ItemResolver() : BaseResolver("ItemResolver") {}
        ~ItemResolver() override = default;

        void visit(ast::FuncDecl * funcDecl) override;

        // Declarations //
    private:
        void declareItem(const std::string & name, Item::Kind kind, ast::node_id nodeId);
    };
}

#endif // JACY_RESOLVE_ITEMRESOLVER_H
