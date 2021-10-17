#ifndef JACY_HIR_HIRPRINTER_H
#define JACY_HIR_HIRPRINTER_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    class HirPrinter {
    public:
        HirPrinter() = default;

        void print(const Party & party);
    };
}

#endif // JACY_HIR_HIRPRINTER_H
