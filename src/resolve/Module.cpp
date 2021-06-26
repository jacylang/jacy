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
        printIndent();
        log.raw("[values]: ", module->valueNS).nl();
        printIndent();
        log.raw("[types]: ", module->typeNS).nl();
        printIndent();
        log.raw("[lifetimes]: ", module->lifetimeNS).nl();
        indent--;
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printIndent() {
        log.raw(utils::str::repeat("  ", indent));
    }
}
