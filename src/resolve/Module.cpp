#include "resolve/Module.h"

namespace jc::resolve {
    // ModulePrinter //
    ModulePrinter::ModulePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModulePrinter::print(module_ptr module) {
        const auto noValues = module->valueNS.empty();
        const auto noTypes = module->typeNS.empty();
        const auto noLifetimes = module->lifetimeNS.empty();
        const auto noChildren = module->children.empty();

        if (noValues and noTypes and noLifetimes and noChildren) {
            log.raw("{}");
            return;
        }

        log.raw("{");
        log.nl();
        indent++;

        for (const auto & child : module->children) {
            printIndent();
            log.raw("[", child.first, "] ");
            print(child.second);
            log.nl();
        }

        if (not noValues) {
            printIndent();
            log.raw("[values]: ", module->valueNS).nl();
        }
        if (not noTypes) {
            printIndent();
            log.raw("[types]: ", module->typeNS).nl();
        }
        if (not noLifetimes) {
            printIndent();
            log.raw("[lifetimes]: ", module->lifetimeNS).nl();
        }
        indent--;
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printIndent() {
        log.raw(utils::str::repeat("  ", indent));
    }
}
