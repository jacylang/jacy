#include "resolve/Importer.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> Importer::declare(sess::Session::Ptr sess, const ast::Party & party) {
        this->sess = sess;

        visitEach(party.items);

        return {None, extractSuggestions()};
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        const auto & useDeclModule = sess->defTable.getUseDeclModule(useDecl.id);

        log.dev("Import `use` with module ", useDeclModule->toString());

        _useDeclModule = useDeclModule;
        _importModule = sess->defTable.getModule(_useDeclModule->nearestModDef);
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
        define(resolvePath(PathResKind::Prefix, useTree.path), useTree.as.unwrap().sym);
    }

    void Importer::visit(const ast::UseTreeAll & useTree) {
        if (useTree.path.some()) {
            resolvePath(PathResKind::Full, useTree.path.unwrap());
        }

        _importModule->perNS.each([&](const Module::NSMap & ns, Namespace nsKind) {
            for (const auto & def : ns) {
                // Note: for `use a::*` we don't report "redefinition" error

                if (def.second.isFuncOverload()) {
                    _useDeclModule->addFuncOverload(def.first, def.second.asFuncOverload());
                } else {
                    _useDeclModule->tryDefine(nsKind, def.first, def.second.asDef());
                }
            }
        });
    }

    // AGENDA: Unify `resolvePath` in `Importer` and in `NameResolver`

    PathResult Importer::resolvePath(PathResKind resKind, const ast::SimplePath & path) {
        std::string pathStr;
        DefKind lastPathSegKind;
        bool inaccessible = false;
        Option<UnresSeg> unresSeg = None;
        DefPerNS defPerNs;

        for (size_t i = 0; i < path.segments.size(); i++) {
            const auto & seg = path.segments.at(i);
            const auto & segName = seg.ident.unwrap().unwrap().sym;

            bool isFirstSeg = i == 0;
            bool isPrefixSeg = i < path.segments.size() - 1;

            if (isPrefixSeg or (path.segments.size() == 1 and isFirstSeg)) {
                while (true) {
                    log.dev("Trying to find '", segName, "' first segment");
                    if (_importModule->hasAny(segName) or _importModule->parent.none()) {
                        break;
                    }
                    _importModule = _importModule->parent.unwrap();
                }
            }

            if (isPrefixSeg or resKind == PathResKind::Full) {
                _importModule->find(Namespace::Type, segName).then([&](const IntraModuleDef & intraModuleDef) {
                    // Agenda: Each function overloading definition has separate visibility, etc.
                    //  Use vector of definitions in `DefPerNS`
                    if (intraModuleDef.isFuncOverload()) {
                        return;
                    }
                    auto defId = intraModuleDef.asDef();
                    if (not isFirstSeg and sess->defTable.getDefVis(defId) != DefVis::Pub) {
                        inaccessible = true;
                        unresSeg = UnresSeg {i, defId};
                        return;
                    }

                    // Only items from type namespace can be descended to
                    _importModule = sess->defTable.getModule(defId);

                    auto defKind = sess->defTable.getDef(defId).kind;
                    lastPathSegKind = defKind;
                    if (not isFirstSeg) {
                        pathStr += "::";
                    }
                    pathStr += segName.toString();

                    if (i == path.segments.size() - 1) {
                        defPerNs.set(Namespace::Type, defId);
                    }
                });

                if (unresSeg.some()) {
                    break;
                }
            } else if (resKind == PathResKind::Prefix) {
                defPerNs = _importModule->findAllDefOnly(segName);

                // Save count of found definitions in module
                // It is useful because
                // - If count is 0 then it is an error
                // - If count is 1 then we report an error if item is private
                // - If count is 2 or more (suppose we had more than 3-4 namespaces) then we skip private items
                uint8_t defsCount = 0;
                uint8_t visDefsCount = 0;
                PerNS<Option<DefVis>> defsPerNSVis{None, None, None};
                defPerNs.each([&](const auto & defs, Namespace nsKind) {
                    for (const auto & defId : defs) {
                        defsCount++;
                        const auto & defVis = sess->defTable.getDefVis(defId);
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
                    defPerNs.each([&](const auto & defs, Namespace nsKind) {
                        for (const auto & defId : defs) {
                            // Report "Cannot access" only if this is the only one inaccessible item
                            if (visDefsCount == 1 and defsPerNSVis.get(nsKind).unwrap() != DefVis::Pub) {
                                inaccessible = true;
                                unresSeg = {i, defId};
                            }
                        }
                    });
                }
            }
        }

        if (unresSeg.some()) {
            // If `pathStr` is empty -- we failed to resolve local variable or item from current module,
            // so give different error message
            const auto & unresolvedSegIdent = path.segments.at(unresSeg.unwrap().segIndex).ident.unwrap().unwrap();
            const auto & unresolvedSegName = unresolvedSegIdent.sym;

            if (inaccessible) {
                const auto & defKind = sess->defTable.getDef(unresSeg.unwrap().defId.unwrap()).kindStr();
                // Report "Cannot access" error
                suggestErrorMsg(
                    log::fmt("Cannot access private ", defKind, " '", unresolvedSegName, "' in '", pathStr, "'"),
                    unresolvedSegIdent.span
                );
            } else {
                // Report "Not defined" error
                auto msg = log::fmt("'", unresolvedSegName, "' is not defined");
                if (not pathStr.empty()) {
                    msg += log::fmt(" in ", Def::kindStr(lastPathSegKind), " '",  pathStr, "'");
                }
                suggestErrorMsg(msg, unresolvedSegIdent.span);
            }
        }

        const auto & lastSeg = path.segments.at(path.segments.size() - 1);

        return PathResult {defPerNs, lastSeg.ident.unwrap().unwrap().sym, lastSeg.span};
    }

    void Importer::define(PathResult && pathResult, const Option<Symbol> & rebind) {
        const auto & segName = pathResult.segName;
        const auto & segSpan = pathResult.segSpan;
        Symbol name = Symbol::empty();
        if (rebind.some()) {
            name = rebind.unwrap();
        } else {
            name = segName;
        }
        pathResult.defPerNs.each([&](const DefId::Opt & optDefId, Namespace nsKind) {
            optDefId.then([&](const auto & defId) {
                _useDeclModule->tryDefine(nsKind, segName, defId).then([&](const IntraModuleDef & intraModuleDef) {
                    if (intraModuleDef.isFuncOverload()) {
                        // TODO!!: Think how to handle function overloads
                        return;
                    }
                    auto oldDefId = intraModuleDef.asDef();
                    // Note: If some definition can be redefined -- it is always named definition,
                    //  so we can safely get its name node span
                    const auto & oldDef = sess->defTable.getDef(oldDefId);
                    const auto & oldDefSpan = sess->defTable.getDefNameSpan(oldDef.defId);
                    suggest(
                        std::make_unique<sugg::MsgSpanLinkSugg>(
                            log::fmt("Cannot `use` '", segName, "'"),
                            segSpan,
                            "Because it is already declared as " + oldDef.kindStr() + " here",
                            oldDefSpan,
                            sugg::SuggKind::Error
                        )
                    );
                });
            });
        });
    }
}
