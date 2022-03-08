#ifndef JACY_SRC_TYPECK_COLLECT_COLLECTOR_H
#define JACY_SRC_TYPECK_COLLECT_COLLECTOR_H

#include "hir/nodes/Party.h"
#include "hir/HirVisitor.h"

namespace jc::typeck {
    class Collector : public hir::HirVisitor {
    public:
        Collector(const hir::Party & party) : hir::HirVisitor {party} {}


    };
}

#endif // JACY_SRC_TYPECK_COLLECT_COLLECTOR_H
