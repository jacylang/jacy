#include "resolve/ModulePrinter.h"

namespace jc::resolve {
    // ModulePrinter //
    ModulePrinter::ModulePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModulePrinter::print(sess::Session::Ptr sess) {
        this->sess = sess;
        printMod(sess->modTreeRoot.unwrap());
    }

    void ModulePrinter::printMod(Module::Ptr module) {
        const auto noValues = module->perNS.value.empty();
        const auto noTypes = module->perNS.type.empty();
        const auto noLifetimes = module->perNS.lifetime.empty();

        const auto & shadowedPrimTypesNames = getShadowedPrimTypes(module->shadowedPrimTypes);

        if (not shadowedPrimTypesNames.empty()) {
            log.raw("(shadows ", shadowedPrimTypesNames, " primitive types) ");
        }

        if (noValues and noTypes and noLifetimes) {
            log.raw("{}");
            return;
        }

        log.raw("{").nl();
        indent++;

        module->perNS.each([&](const Module::NSMap & ns, Namespace nsKind) {
            for (const auto & [name, defId] : ns) {
                printIndent();
                log.raw("'", name, "' (", Module::nsToString(nsKind), ") ");
                printDef(defId);
                log.nl();
            }
        });

        indent--;
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printDef(const DefId & defId) {
        const auto & def = sess->defStorage.getDef(defId);
        log.raw(def.kindStr());

        if (def.nameNodeId.some()) {
            log.raw(" (", def.nameNodeId.unwrap(), ")");
        }

        switch (def.kind) {
            case DefKind::Enum:
            case DefKind::Impl:
            case DefKind::Mod:
            case DefKind::Struct:
            case DefKind::Func:
            case DefKind::Trait: {
                log.raw(" ");
                printMod(sess->defStorage.getModule(defId));
                break;
            }
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
