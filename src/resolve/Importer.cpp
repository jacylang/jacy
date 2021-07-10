#include "resolve/Importer.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> Importer::declare(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        return {dt::None, extractSuggestions()};
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        _module = sess->defStorage.getUseDeclModule(useDecl.id);
        useDecl.useTree.accept(*this);
    }

    void Importer::visit(const ast::UseTreeRaw & useTree) {
        std::string pathStr;
        module_ptr searchMod = _module;
        for (size_t i = 0; i < useTree.path->segments.size(); i++) {
            const auto & seg = useTree.path->segments.at(i);
            const auto & segName = seg->ident.unwrap().unwrap()->getValue();
            const auto isPrefixSeg = i < useTree.path->segments.size() - 1;

            if (isPrefixSeg) {
                const auto & defId = searchMod->find(Namespace::Type, segName);
                // Only items from type namespace can be descended to
                searchMod = sess->defStorage.getModule(defId);
            } else {
                auto defsPerNS = searchMod->findAll(segName);

                if (defsPerNS.type.none() and defsPerNS.value.none() and defsPerNS.lifetime.none()) {
                    auto msg = "Cannot find '" + segName + "'";
                    if (not pathStr.empty()) {
                        msg += " in '" + pathStr + "'";
                    }
                    suggestErrorMsg(msg, seg->span);
                } else {
                    defsPerNS.each([&](opt_def_id optDefId, Namespace nsKind) {
                        optDefId.then([&](def_id defId) {
                            _module->tryDefine(nsKind, segName, defId).then([&](def_id oldDefId) {
                                suggestErrorMsg(
                                    "Cannot `use` '" + segName + "' as it was already declared in this scope",
                                    seg->span);
                            });
                        });
                    });
                }
            }
        }
    }
}
