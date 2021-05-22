#ifndef JACY_RESOLVE_ITEMRESOLVER_H
#define JACY_RESOLVE_ITEMRESOLVER_H

#include "resolve/BaseResolver.h"

namespace jc::resolve {
    class ItemResolver : public BaseResolver {
    public:
        ItemResolver(sess::sess_ptr sess) : BaseResolver("ItemResolver", sess) {}
        ~ItemResolver() override = default;

        void visit(ast::FuncDecl * funcDecl) override;
        void visit(ast::Trait * trait) override;

        // Declarations //
    private:
        void declareItem(const std::string & name, Item::Kind kind, ast::node_id nodeId);
    };
}

#endif // JACY_RESOLVE_ITEMRESOLVER_H
