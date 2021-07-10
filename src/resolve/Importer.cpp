#include "resolve/Importer.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> Importer::declare(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        return {None, extractSuggestions()};
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        auto prevModule = _module;
        const auto & useDeclModule = sess->defStorage.getUseDeclModule(useDecl.id);

        log.dev("Import `use` with module ", useDeclModule->toString());

        _module = useDeclModule;
        useDecl.useTree.accept(*this);
        _module = prevModule;
    }

    void Importer::visit(const ast::UseTreeRaw & useTree) {
        // TODO!!!: Unify path resolution logic in NameResolver and Importer,
        //  - `SimplePath` must be removed and only `Path` will be used, thus we don't have two path kinds

        module_ptr searchMod = sess->defStorage.getModule(_module->nearestModDef.unwrap());

        std::string pathStr;
        bool inaccessible = false;
        dt::Option<UnresSeg> unresSeg{None};
        PerNS<opt_def_id> altDefs{None, None, None};

        for (size_t i = 0; i < useTree.path->segments.size(); i++) {
            const auto & seg = useTree.path->segments.at(i);
            const auto & segName = seg->ident.unwrap().unwrap()->getValue();

            bool isFirstSeg = i == 0;
            bool isPrefixSeg = i < useTree.path->segments.size() - 1;

            if (isPrefixSeg) {
                const auto & defId = searchMod->find(Namespace::Type, segName);
                // Only items from type namespace can be descended to
                searchMod = sess->defStorage.getModule(defId.unwrap());
                if (i != 0) {
                    pathStr += "::";
                }
                pathStr += segName;
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
                                // Note: If some definition can be redefined -- it is always named definition,
                                //  so we can safely get its name node span
                                const auto & oldDef = sess->defStorage.getDef(oldDefId);
                                const auto & oldDefSpan = sess->nodeMap.getNodeSpan(oldDef.nameNodeId.unwrap());
                                suggest(
                                    std::make_unique<sugg::MsgSpanLinkSugg>(
                                        "Cannot `use` '" + segName + "'",
                                        seg->span,
                                        "Because it is already declared as " + oldDef.kindStr() + " here",
                                        oldDefSpan,
                                        sugg::SuggKind::Error));
                            });
                        });
                    });
                }
            }
        }
    }
}
