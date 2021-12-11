#include "hir/HirPrinter.h"

namespace jc::hir {
    HirPrinter::HirPrinter(Party & party) : party {party} {}

    void HirPrinter::print() {
        printMod(party.rootMod());
    }

    void HirPrinter::printMod(const Mod & mod) {
        for (const auto & itemId : mod.items) {
            printItem(itemId);
        }
    }

    void HirPrinter::printItem(const ItemId & itemId) {
        const auto & item = party.item(itemId);

        printVis(item.vis);

        switch (item.item->kind) {
            case ItemKind::Enum: {

                break;
            }
            case ItemKind::Func:
                break;
            case ItemKind::Impl:
                break;
            case ItemKind::Mod:
                break;
            case ItemKind::Struct:
                break;
            case ItemKind::Trait:
                break;
            case ItemKind::TypeAlias:
                break;
            case ItemKind::Use:
                break;
        }
    }

    void HirPrinter::printVis(Item::Vis vis) {
        if (vis.kind == ast::VisKind::Pub) {
            log.raw("pub ");
        }
    }
}
