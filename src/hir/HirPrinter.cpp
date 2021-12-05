#include "hir/HirPrinter.h"

namespace jc::hir {
    HirPrinter::HirPrinter(Party & party) : party {party} {}

    void HirPrinter::print() {

    }

    void HirPrinter::printMod(const Mod & mod) {
        for (const auto & itemId : mod.items) {
            printItem(itemId);
        }
    }

    void HirPrinter::printItem(const ItemId & itemId) {
        const auto & item = party.item(itemId);

        switch (item.item->kind) {

        }
    }
}
