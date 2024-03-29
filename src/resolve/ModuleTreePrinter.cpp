#include "resolve/ModuleTreePrinter.h"

namespace jc::resolve {
    ModuleTreePrinter::ModuleTreePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModuleTreePrinter::print(sess::Session::Ptr sess) {
        this->sess = sess;
        log.raw(
            "Print style: 'Name in Module' (Namespace): ",
            "`Definition kind` 'Definition full name' #defId {Inner definitions}"
        ).nl();
        printMod(sess->modTreeRoot.unwrap());

        log.nl();
        log.raw("Blocks:").nl();
        for (const auto & block : sess->defTable.getBlocks()) {
            log.raw(block.first, ": ");
            printMod(block.second);
            log.nl();
        }
    }

    void ModuleTreePrinter::printMod(Module::Ptr module) {
        const auto noValues = module->perNS.value.empty();
        const auto noTypes = module->perNS.type.empty();
        const auto noLifetimes = module->perNS.lifetime.empty();

        const auto & shadowedPrimTypesNames = getShadowedPrimTypes(module->shadowedPrimTypes);

        if (not shadowedPrimTypesNames.empty()) {
            log.raw("(shadows ", shadowedPrimTypesNames, " primitive types) ");
        }

        // Useless print, as we already print definition id
//        log.raw(module->toString(), " {");
        log.raw("{");

        if (noValues and noTypes and noLifetimes) {
            log.raw("}");
            return;
        } else {
            log.nl();
        }

        indent++;
        module->perNS.each([&](const Module::NSMap & ns, Namespace nsKind) {
            for (const auto & [name, def] : ns) {
                printIndent();
                log.raw("'", name, "' (", nsToString(nsKind), "): ");
                printNameBinding(def);
                log.nl();
            }
        });
        indent--;

        printIndent();
        log.raw("}");
    }

    void ModuleTreePrinter::printNameBinding(const NameBinding & nameBinding) {
        if (nameBinding.isFOS()) {
            printFOS(nameBinding.asFOS());
        } else {
            printDef(nameBinding.asDef());
        }
    }

    void ModuleTreePrinter::printDef(const DefId & defId) {
        const auto & def = sess->defTable.getDef(defId);
        const auto defVis = sess->defTable.getDefVis(defId);

        log.raw(Def::visStr(defVis));
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
            case DefKind::ImportAlias: {
                log.raw("Import Alias ", defId);
                printDef(sess->defTable.getImportAlias(defId));
                break;
            }
            case DefKind::Struct:
            case DefKind::Lifetime:
            case DefKind::TypeAlias:
            case DefKind::TypeParam:
            case DefKind::Const:
            case DefKind::ConstParam:
            case DefKind::Variant:
            case DefKind::DefaultInit: {
                break;
            }
        }
    }

    void ModuleTreePrinter::printFOS(const FOSId & fosId) {
        log.raw(fosId, " ");
        const auto & fos = sess->defTable.getFOS(fosId);
        if (fos.size() == 1) {
            printDef(fos.begin()->second);
        } else if (not fos.empty()) {
            indent++;
            for (const auto & overload : fos) {
                log.nl();
                printIndent();
                log.raw("- ");
                printDef(overload.second);
            }
            indent--;
        }
    }

    void ModuleTreePrinter::printIndent() {
        log.raw(utils::str::repeat("  ", indent));
    }
}
