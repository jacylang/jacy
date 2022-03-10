#ifndef JACY_SRC_TYPECK_COLLECT_ITEMCOLLECTOR_H
#define JACY_SRC_TYPECK_COLLECT_ITEMCOLLECTOR_H

#include "hir/nodes/Party.h"
#include "hir/HirVisitor.h"
#include "session/Session.h"

namespace jc::typeck {
    class ItemCollector : public hir::HirVisitor {
    public:
        ItemCollector(const hir::Party & party, const sess::Session::Ptr & sess) : hir::HirVisitor {party}, sess {sess} {}



    private:
        sess::Session::Ptr sess;
    };
}

#endif // JACY_SRC_TYPECK_COLLECT_ITEMCOLLECTOR_H
