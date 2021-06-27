#include "resolve/Module.h"
#include "session/Session.h"

namespace jc::resolve {
    // ModulePrinter //
    ModulePrinter::ModulePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModulePrinter::print(sess::sess_ptr sess) {
        this->sess = sess;
        printMod(sess->modTreeRoot.unwrap());
    }

    void ModulePrinter::printMod(module_ptr module) {
        const auto noValues = module->valueNS.empty();
        const auto noTypes = module->typeNS.empty();
        const auto noLifetimes = module->lifetimeNS.empty();

        const auto & shadowedPrimTypesNames = getShadowedPrimTypes(module->shadowedPrimTypes);

        if (not shadowedPrimTypesNames.empty()) {
            log.raw("(shadows ", shadowedPrimTypesNames, " primitive types) ");
        }

        if (noValues and noTypes and noLifetimes) {
            log.raw("{}");
            return;
        }

        log.raw("{");
        log.nl();
        indent++;

        if (not noValues) {
            printIndent();
            log.raw("[value NS]: ");
            printNS(module->valueNS);
            log.nl();
        }
        if (not noTypes) {
            printIndent();
            log.raw("[type NS]: ");
            printNS(module->typeNS);
            log.nl();
        }
        if (not noLifetimes) {
            printIndent();
            log.raw("[lifetime NS]: ");
            printNS(module->lifetimeNS);
            log.nl();
        }

        indent--;
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printNS(const mod_ns_map & ns) {
        for (const auto & [name, defId] : ns) {
            log.raw("'", name, "' ");
            printDef(defId);
        }
    }

    void ModulePrinter::printDef(def_id defId) {
        const auto & def = sess->defStorage.getDef(defId);
        log.raw(def.kindStr());

        switch (def.kind) {
            case DefKind::Dir:
            case DefKind::File:
            case DefKind::Root:
            case DefKind::Enum:
            case DefKind::Impl:
            case DefKind::Mod:
            case DefKind::Struct:
            case DefKind::Trait: {
                log.raw(" ");
                printMod(sess->defStorage.getModule(defId));
                break;
            }
            case DefKind::Func:
            case DefKind::Lifetime:
            case DefKind::TypeAlias:
            case DefKind::TypeParam:
            case DefKind::Const:
            case DefKind::ConstParam:
            case DefKind::Variant:
                break;
        }
    }

    void ModulePrinter::printIndent() {
        log.raw(utils::str::repeat("  ", indent));
    }
}
