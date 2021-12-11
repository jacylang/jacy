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
        void printVis(Item::Vis vis);

    private:
        Party & party;

    private:
        log::Logger log {"hir-printer"};
    };
}

#endif // JACY_HIR_HIRPRINTER_H
