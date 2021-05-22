#ifndef JACY_RESOLVE_ITEMRESOLVER_H
#define JACY_RESOLVE_ITEMRESOLVER_H

#include "resolve/BaseResolver.h"

namespace jc::resolve {
    class ItemResolver : public BaseResolver {
    public:
        ItemResolver() : BaseResolver("ItemResolver") {}
        ~ItemResolver() override = default;

        void visit(ast::FuncDecl * funcDecl) override;

        // Ribs //
    private:
        rib_ptr rib;
        void acceptRib(rib_ptr newRib);

        void declareItem(const std::string & name, item_ptr item);
    };
}

#endif // JACY_RESOLVE_ITEMRESOLVER_H
