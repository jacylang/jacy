#ifndef JACY_SRC_TYPECK_COLLECT_ITEMCOLLECTOR_H
#define JACY_SRC_TYPECK_COLLECT_ITEMCOLLECTOR_H

#include "hir/nodes/Party.h"
#include "hir/HirVisitor.h"
#include "session/Session.h"

namespace jc::typeck {
    class ItemCollector : public hir::HirVisitor {
    public:
        ItemCollector(const hir::Party & party, const sess::Session::Ptr & sess)
            : hir::HirVisitor {party},
              sess {sess},
              tyCtx {sess->tyCtx} {}

        virtual ~ItemCollector() = default;

        void visitFunc(const hir::Func & func, const hir::Item::ItemData & data) override;

    private:
        sess::Session::Ptr sess;
        TypeContext & tyCtx;
    };
}

#endif // JACY_SRC_TYPECK_COLLECT_ITEMCOLLECTOR_H
