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
        // TODO!!!: Unify path resolution logic in NameResolver and Importer. It might be impossible btw

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
                searchMod->find(Namespace::Type, segName).then([&](def_id defId) {
                    if (not isFirstSeg and sess->defStorage.getDefVis(defId) != DefVis::Pub) {
                        inaccessible = true;
                        unresSeg = {i, defId};
                        return;
                    }

                    // Only items from type namespace can be descended to
                    searchMod = sess->defStorage.getModule(defId);
                    if (not isFirstSeg) {
                        pathStr += "::";
                    }
                    pathStr += segName;
                });

                if (unresSeg) {
                    break;
                }
            } else {
                auto defsPerNS = searchMod->findAll(segName);

                // Save count of found definitions in module
                // It is useful because
                // - If count is 0 then it is an error
                // - If count is 1 then we report an error if item is private
                // - If count is 2 or more (suppose we had more than 3-4 namespaces) then we skip private items
                uint8_t defsCount = 0;
                PerNS<dt::Option<DefVis>> defsPerNSVis{None, None, None};
                defsPerNS.each([&](opt_def_id optDefId, Namespace nsKind) {
                    if (optDefId.some()) {
                        defsCount++;
                        const auto & defVis = sess->defStorage.getDefVis(optDefId.unwrap());
                        defsPerNSVis.set(nsKind, defVis);
                    }
                });

                if (defsCount == 0) {
                    // No target found
                    unresSeg = {i, None};
                } else {
                    defsPerNS.each([&](opt_def_id optDefId, Namespace nsKind) {
                        optDefId.then([&](def_id defId) {
                            if (defsCount == 1) {
                                inaccessible = true;
                                unresSeg = {i, defId};
                            }
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

        if (unresSeg) {
            // If `pathStr` is empty -- we failed to resolve local variable or item from current module,
            // so give different error message
            const auto & unresolvedSegIdent = useTree.path->segments.at(unresSeg.unwrap().segIndex)->ident
                                                .unwrap().unwrap();
            const auto & unresolvedSegName = unresolvedSegIdent->getValue();

            if (inaccessible) {
                const auto & defKind = sess->defStorage.getDef(unresSeg.unwrap().defId.unwrap()).kindStr();
                // Report "Cannot access" error
                suggestErrorMsg(
                    "Cannot access private " + defKind + " '" + unresolvedSegName + "' in '" + pathStr + "'",
                    unresolvedSegIdent->span);
            } else {
                // Report "Not defined" error
                auto msg = "'" + unresolvedSegName + "' is not defined";
                if (not pathStr.empty()) {
                    msg += " in '" + pathStr + "'";
                }
                suggestErrorMsg(msg, unresolvedSegIdent->span);
            }
        }
    }
}
