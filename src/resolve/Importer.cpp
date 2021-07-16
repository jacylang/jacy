#include "resolve/Importer.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> Importer::declare(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        party.getRootFile().accept(*this);
        party.getRootDir().accept(*this);

        return {None, extractSuggestions()};
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        const auto & useDeclModule = sess->defStorage.getUseDeclModule(useDecl.id);

        log.dev("Import `use` with module ", useDeclModule->toString());

        _useDeclModule = useDeclModule;
        _importModule = sess->defStorage.getModule(_useDeclModule->nearestModDef.unwrap());
        useDecl.useTree.autoAccept(*this);
    }

    void Importer::visit(const ast::UseTreeRaw & useTree) {
        // TODO!!!: Unify path resolution logic in NameResolver and Importer. It might be impossible btw.
        // TODO!!!: `pub use...` re-exporting, now all `use`s are public

        define(resolvePath(PathResKind::Prefix, useTree.path), None);
    }

    void Importer::visit(const ast::UseTreeSpecific & useTree) {
        // If path given -- descend to module it points to
        if (useTree.path.some()) {
            resolvePath(PathResKind::Full, useTree.path.unwrap());
        }

        // Here, we resolve specifics relatively to current path
        for (const auto & specific : useTree.specifics) {
            specific.autoAccept(*this);
        }
    }

    void Importer::visit(const ast::UseTreeRebind & useTree) {
        define(resolvePath(PathResKind::Prefix, useTree.path), useTree.as.unwrap().getValue());
    }

    void Importer::visit(const ast::UseTreeAll & useTree) {
        if (useTree.path.some()) {
            resolvePath(PathResKind::Full, useTree.path.unwrap());
        }

        _importModule->perNS.each([&](const mod_ns_map & ns, Namespace nsKind) {
            for (const auto & def : ns) {
                // Note: for `use a::*` we don't report "redefinition" error
                _useDeclModule->tryDefine(nsKind, def.first, def.second);
            }
        });
    }

    PathResult Importer::resolvePath(PathResKind resKind, const ast::SimplePath & path) {
        std::string pathStr;
        bool inaccessible = false;
        Option<UnresSeg> unresSeg{None};
        DefPerNS defPerNs{None, None, None};

        for (size_t i = 0; i < path.segments.size(); i++) {
            const auto & seg = path.segments.at(i);
            const auto & segName = seg.ident.unwrap().unwrap().getValue();

            bool isFirstSeg = i == 0;
            bool isPrefixSeg = i < path.segments.size() - 1;

            if (isPrefixSeg or resKind == PathResKind::Full) {
                _importModule->find(Namespace::Type, segName).then([&](const auto & defId) {
                    if (not isFirstSeg and sess->defStorage.getDefVis(defId) != DefVis::Pub) {
                        inaccessible = true;
                        unresSeg = {i, defId};
                        return;
                    }

                    // Only items from type namespace can be descended to
                    _importModule = sess->defStorage.getModule(defId);
                    if (not isFirstSeg) {
                        pathStr += "::";
                    }
                    pathStr += segName;

                    if (i == path.segments.size() - 1) {
                        defPerNs.set(Namespace::Type, defId);
                    }
                });

                if (unresSeg.some()) {
                    break;
                }
            } else if (resKind == PathResKind::Prefix) {
                defPerNs = _importModule->findAll(segName);

                // Save count of found definitions in module
                // It is useful because
                // - If count is 0 then it is an error
                // - If count is 1 then we report an error if item is private
                // - If count is 2 or more (suppose we had more than 3-4 namespaces) then we skip private items
                uint8_t defsCount = 0;
                uint8_t visDefsCount = 0;
                PerNS<Option<DefVis>> defsPerNSVis{None, None, None};
                defPerNs.each([&](opt_def_id optDefId, Namespace nsKind) {
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
                    defPerNs.each([&](opt_def_id optDefId, Namespace nsKind) {
                        optDefId.then([&](const auto & defId) {
                            // Report "Cannot access" only if this is the only one inaccessible item
                            if (visDefsCount == 1 and defsPerNSVis.get(nsKind).unwrap() != DefVis::Pub) {
                                inaccessible = true;
                                unresSeg = {i, defId};
                            }
                        });
                    });
                }
            }
        }

        if (unresSeg.some()) {
            // If `pathStr` is empty -- we failed to resolve local variable or item from current module,
            // so give different error message
            const auto & unresolvedSegIdent = path.segments.at(unresSeg.unwrap().segIndex).ident.unwrap().unwrap();
            const auto & unresolvedSegName = unresolvedSegIdent.getValue();

            if (inaccessible) {
                const auto & defKind = sess->defStorage.getDef(unresSeg.unwrap().defId.unwrap()).kindStr();
                // Report "Cannot access" error
                suggestErrorMsg(
                    "Cannot access private " + defKind + " '" + unresolvedSegName + "' in '" + pathStr + "'",
                    unresolvedSegIdent.span);
            } else {
                // Report "Not defined" error
                auto msg = "'" + unresolvedSegName + "' is not defined";
                if (not pathStr.empty()) {
                    msg += " in '" + pathStr + "'";
                }
                suggestErrorMsg(msg, unresolvedSegIdent.span);
            }
        }

        const auto & lastSeg = path.segments.at(path.segments.size() - 1);

        return PathResult{defPerNs, lastSeg.ident.unwrap().unwrap().getValue(), lastSeg.span};
    }

    void Importer::define(PathResult && pathResult, const Option<std::string> & rebind) {
        const auto & segName = pathResult.segName;
        const auto & segSpan = pathResult.segSpan;
        std::string name;
        if (rebind.some()) {
            name = rebind.unwrap();
        } else {
            name = segName;
        }
        pathResult.defPerNs.each([&](const opt_def_id & optDefId, Namespace nsKind) {
            optDefId.then([&](const auto & defId) {
                _useDeclModule->tryDefine(nsKind, segName, defId).then([&](const auto & oldDefId) {
                    // Note: If some definition can be redefined -- it is always named definition,
                    //  so we can safely get its name node span
                    const auto & oldDef = sess->defStorage.getDef(oldDefId);
                    const auto & oldDefSpan = sess->nodeStorage.getNodeSpan(oldDef.nameNodeId.unwrap());
                    suggest(
                        std::make_unique<sugg::MsgSpanLinkSugg>(
                            "Cannot `use` '" + segName + "'",
                            segSpan,
                            "Because it is already declared as " + oldDef.kindStr() + " here",
                            oldDefSpan,
                            sugg::SuggKind::Error));
                });
            });
        });
    }
}
