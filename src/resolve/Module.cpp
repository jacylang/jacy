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
        const auto noValues = module->perNS.value.empty();
        const auto noTypes = module->perNS.type.empty();
        const auto noLifetimes = module->perNS.lifetime.empty();

        const auto & shadowedPrimTypesNames = getShadowedPrimTypes(module->shadowedPrimTypes);

        const auto & moduleDef = sess->defStorage.getDef(module->defId);
        if (moduleDef.nameNodeId) {
            log.raw("(#", moduleDef.nameNodeId.unwrap(), ") ");
        }

        if (not shadowedPrimTypesNames.empty()) {
            log.raw("(shadows ", shadowedPrimTypesNames, " primitive types) ");
        }

        if (noValues and noTypes and noLifetimes) {
            log.raw("{}");
            return;
        }

        log.raw("{").nl();
        indent++;

        if (not noValues) {
            printIndent();
            log.raw("[value NS]: ");
            printNS(module->perNS.value);
            log.nl();
        }
        if (not noTypes) {
            printIndent();
            log.raw("[type NS]: ");
            printNS(module->perNS.type);
            log.nl();
        }
        if (not noLifetimes) {
            printIndent();
            log.raw("[lifetime NS]: ");
            printNS(module->perNS.lifetime);
            log.nl();
        }

        indent--;
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printNS(const mod_ns_map & ns) {
        if (ns.empty()) {
            log.raw("{}");
            return;
        }
        log.raw("{").nl();
        indent++;
        for (const auto & [name, defId] : ns) {
            printIndent();
            log.raw("'", name, "' ");
            printDef(defId);
            log.nl();
        }
        indent--;
        printIndent();
        log.raw("}");
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
