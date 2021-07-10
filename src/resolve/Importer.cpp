#include "resolve/Importer.h"

namespace jc::resolve {
    void Importer::declare(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        _module = sess->defStorage.getUseDeclModule(useDecl.id);
        useDecl.useTree.accept(*this);
    }

    void Importer::visit(const ast::UseTreeRaw & useTree) {
        std::string pathStr;
        module_ptr searchMod = _module;
        for (const auto & seg : useTree.path->segments) {
            const auto & segName = seg->ident.unwrap().unwrap()->getValue();
            auto allDefs = searchMod->findAll(segName);
            opt_def_id defId{dt::None};
            allDefs.each([&](opt_def_id optDefId, Namespace nsKind) {
                if (optDefId) {
                    if (defId) {
                        log.devPanic("Found multiple definitions in module");
                    }
                    defId = optDefId.unwrap();
                }
            });

            if (not defId) {
                auto msg = "Cannot find '" + segName + "'";
                if (not pathStr.empty()) {
                    msg += " in '" + pathStr + "'";
                }
                suggestErrorMsg(msg, seg->span);
            } else {
                searchMod = sess->defStorage.getModule(defId.unwrap());
            }
        }
    }
}
