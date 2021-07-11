#include "resolve/Importer.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> Importer::declare(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        return {None, extractSuggestions()};
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        const auto & useDeclModule = sess->defStorage.getUseDeclModule(useDecl.id);

        log.dev("Import `use` with module ", useDeclModule->toString());

        _useDeclModule = useDeclModule;
        useDecl.useTree.accept(*this);
    }

    void Importer::visit(const ast::UseTreeRaw & useTree) {
        // TODO!!!: Unify path resolution logic in NameResolver and Importer. It might be impossible btw
        // TODO!!!: `pub use...` reexports, now all `use`s are public

        resolvePath(PathResKind::Prefix, *useTree.path, [&](DefPerNS defPerNs) {
            defPerNs.each([&](const opt_def_id & optDefId, Namespace nsKind) {
                optDefId.then([&](def_id defId) {
                    _useDeclModule->tryDefine(nsKind, segName, defId).then([&](def_id oldDefId) {
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
        });
    }

    void Importer::visit(const ast::UseTreeSpecific & useTree) {
    }

    void Importer::resolvePath(
        PathResKind resKind,
        const ast::SimplePath & path,
        const PathResCB & cb
    ) {
        module_ptr searchMod = sess->defStorage.getModule(_useDeclModule->nearestModDef.unwrap());

        std::string pathStr;
        bool inaccessible = false;
        dt::Option<UnresSeg> unresSeg{None};

        for (size_t i = 0; i < path.segments.size(); i++) {
            const auto & seg = path.segments.at(i);
            const auto & segName = seg->ident.unwrap().unwrap()->getValue();

            bool isFirstSeg = i == 0;
            bool isPrefixSeg = i < path.segments.size() - 1;

            if (isPrefixSeg or resKind == PathResKind::Full) {
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

                    if (i == path.segments.size() - 1) {
                        DefPerNS defPerNs{None, None, None};
                        defPerNs.set(Namespace::Type, defId);
                        cb(defPerNs);
                    }
                });

                if (unresSeg) {
                    break;
                }
            } else if (resKind == PathResKind::Prefix) {
                auto defsPerNS = searchMod->findAll(segName);

                // Save count of found definitions in module
                // It is useful because
                // - If count is 0 then it is an error
                // - If count is 1 then we report an error if item is private
                // - If count is 2 or more (suppose we had more than 3-4 namespaces) then we skip private items
                uint8_t defsCount = 0;
                uint8_t visDefsCount = 0;
                PerNS<dt::Option<DefVis>> defsPerNSVis{None, None, None};
                defsPerNS.each([&](opt_def_id optDefId, Namespace nsKind) {
                    if (optDefId.some()) {
                        defsCount++;
                        const auto & defVis = sess->defStorage.getDefVis(optDefId.unwrap());
                        defsPerNSVis.set(nsKind, defVis);
                        if (defVis == DefVis::Pub) {
                            visDefsCount++;
                        }
                    }
                });

                if (defsCount == 0) {
                    // No target found
                    unresSeg = {i, None};
                } else {
                    // Note: We check visibility for definitions
                    //  and invoke callback even if some definitions are private
                    defsPerNS.each([&](opt_def_id optDefId, Namespace nsKind) {
                        optDefId.then([&](def_id defId) {
                            // Report "Cannot access" only if this is the only one inaccessible item
                            if (visDefsCount == 1 and defsPerNSVis.get(nsKind).unwrap() != DefVis::Pub) {
                                inaccessible = true;
                                unresSeg = {i, defId};
                            }
                        });
                    });

                    cb(defsPerNS);
                }
            }
        }

        if (unresSeg) {
            // If `pathStr` is empty -- we failed to resolve local variable or item from current module,
            // so give different error message
            const auto & unresolvedSegIdent = path.segments.at(unresSeg.unwrap().segIndex)->ident.unwrap().unwrap();
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
