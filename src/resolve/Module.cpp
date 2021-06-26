#include "resolve/Module.h"

namespace jc::resolve {
    // ModulePrinter //
    ModulePrinter::ModulePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModulePrinter::print(module_ptr module) {
        log.raw("{");
        log.nl();
        indent++;
        for (const auto & child : module->children) {
            printIndent();
            log.raw("[", child.first, "] ");
            print(child.second);
            log.nl();
        }
        if (not module->valueNS.empty()) {
            printIndent();
            log.raw("[values]: ", module->valueNS).nl();
        }
        if (not module->typeNS.empty()) {
            printIndent();
            log.raw("[types]: ", module->typeNS).nl();
        }
        if (not module->lifetimeNS.empty()) {
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
