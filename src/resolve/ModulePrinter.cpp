#include "resolve/ModulePrinter.h"

namespace jc::resolve {
    // ModulePrinter //
    ModulePrinter::ModulePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModulePrinter::print(sess::Session::Ptr sess) {
        this->sess = sess;
        log.raw(
            "Print style: 'Name in Module' (Namespace): ",
            "`Definition kind` 'Definition full name' #defId {Inner definitions}"
        ).nl();
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
            for (const auto & [name, def] : ns) {
                printIndent();
                log.raw("'", name, "' (", Module::nsToString(nsKind), "): ");
                printDef(def);
                log.nl();
            }
        });

        indent--;
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printDef(const IntraModuleDef & intraModuleDef) {
        if (intraModuleDef.isFuncOverload()) {
            auto overloadId = intraModuleDef.asFuncOverload();
            for (const auto & overload : sess->defTable.getFuncOverload(overloadId)) {
                printDef(overload.second);
                log.nl();
            }
        } else if (intraModuleDef.isTarget()) {
            auto defId = intraModuleDef.asDef();
            const auto & def = sess->defTable.getDef(defId);
            log.raw(def);

            switch (def.kind) {
                case DefKind::Enum:
                case DefKind::Impl:
                case DefKind::Mod:
                case DefKind::Struct:
                case DefKind::Func:
                case DefKind::Init:
                case DefKind::Trait: {
                    log.raw(" ");
                    printMod(sess->defTable.getModule(defId));
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
        } else {
            log::devPanic("Unhandled `IntraModuleDef` kind in `ModulePrinter::printDef`");
        }
    }

    void ModulePrinter::printIndent() {
        log.raw(utils::str::repeat("  ", indent));
    }
}
