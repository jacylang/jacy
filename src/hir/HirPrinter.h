#ifndef JACY_HIR_HIRPRINTER_H
#define JACY_HIR_HIRPRINTER_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    class HirPrinter {
    public:
        HirPrinter(Party & party);

        void print();

    private:
        void printMod(const Mod & mod);
        void printItem(const ItemId & itemId);

    private:
        Party & party;
    };
}

#endif // JACY_HIR_HIRPRINTER_H
