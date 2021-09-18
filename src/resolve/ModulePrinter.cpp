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
                printNameBinding(def);
                log.nl();
            }
        });

        indent--;
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printNameBinding(const NameBinding & nameBinding) {
        if (nameBinding.isFOS()) {
            printFOS(nameBinding.asFOS());
        } else {
            printDef(nameBinding.asDef());
        }
    }

    void ModulePrinter::printDef(const DefId & defId) {
        const auto & def = sess->defTable.getDef(defId);
        const auto defVis = sess->defTable.getDefVis(defId);

        switch (defVis) {
            case Vis::Unset:;
            case Vis::Pub: {
                log.raw("pub ");
            }
        }

        log.raw(def);

        switch (def.kind) {
            case DefKind::Enum:
            case DefKind::Impl:
            case DefKind::Mod:
            case DefKind::Func:
            case DefKind::Init:
            case DefKind::Trait: {
                log.raw(" ");
                printMod(sess->defTable.getModule(defId));
                break;
            }
            case DefKind::Struct:
            case DefKind::Lifetime:
            case DefKind::TypeAlias:
            case DefKind::TypeParam:
            case DefKind::Const:
            case DefKind::ConstParam:
            case DefKind::Variant:
                break;
        }
    }

    void ModulePrinter::printFOS(const FOSId & fosId) {
        log.raw(fosId);
        const auto & fos = sess->defTable.getFOS(fosId);
        if (fos.size() == 1) {
            printDef(fos.begin()->second);
        } else if (not fos.empty()) {
            for (const auto & overload : fos) {
                log.nl();
                printIndent();
                log.raw("- ");
                printDef(overload.second);
            }
        }
    }

    void ModulePrinter::printIndent() {
        log.raw(utils::str::repeat("  ", indent));
    }
}
