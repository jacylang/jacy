#include "resolve/Importer.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> Importer::declare(sess::Session::Ptr sess, const ast::Party & party) {
        this->sess = sess;
        pathResolver.init(sess);

        visitEach(party.items);

        // `PathResolver` has its own suggestion collection, thus we need to extract all of them
        return {None, utils::arr::moveConcat(extractSuggestions(), pathResolver.extractSuggestions())};
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        // Module to import items to
        const auto & useDeclModule = sess->defTable.getUseDeclModule(useDecl.id);
        _useDeclModule = useDeclModule;

        log.dev("Import `use` to module ", useDeclModule->toString());

        // Module to search item in
        _importModule = sess->defTable.getModule(_useDeclModule->nearestModDef);

        useDecl.useTree.autoAccept(*this);
    }

    void Importer::visit(const ast::UseTree & useTree) {
        if (useTree.kind == ast::UseTree::Kind::Rebind or useTree.path.some()) {
            useTree.path.unwrap().accept(*this);
        }

        switch (useTree.kind) {
            case ast::UseTree::Kind::Raw: {
                auto res = pathResolver.resolve(
                    _importModule, Namespace::Any, useTree.path.unwrap(), None, ResMode::Import
                );
                if (res.ok()) {
                    import(res.asImport(), useTree.path.unwrap(), None);
                }
                break;
            }
            case ast::UseTree::Kind::Rebind: {
                auto res = pathResolver.resolve(
                    _importModule, Namespace::Any, useTree.path.unwrap(), dt::None, ResMode::Import
                );
                if (res.ok()) {
                    import(res.asImport(), useTree.path.unwrap(), useTree.expectRebinding().sym);
                }
                break;
            }
            case ast::UseTree::Kind::All: {
                if (descendByPath(useTree.path)) {
                    _importModule->perNS.each([&](const Module::NSMap & ns, Namespace nsKind) {
                        for (const auto & def : ns) {
                            // Note: for `use a::*` we don't report "redefinition" error

                            const auto & span = useTree.path.unwrap().span;
                            const auto & nodeId = useTree.path.unwrap().getNodeId();
                            const auto & name = def.first;
                            const auto & nameBinding = def.second;

                            if (nameBinding.isFOS()) {
                                defineFOSImportAlias(Vis::Pub, nodeId, nameBinding.asFOS(), name, span);
                            } else {
                                defineImportAlias(nsKind, nodeId, Vis::Pub, nameBinding.asDef(), name, span);
                            }
                        }
                    });
                }
                break;
            }
            case ast::UseTree::Kind::Specific: {
                if (descendByPath(useTree.path)) {
                    // Here, we resolve specifics relatively to current path
                    for (const auto & specific : useTree.expectSpecifics()) {
                        specific.autoAccept(*this);
                    }
                }
                break;
            }
        }
    }

    /**
     * @brief Descends to the module by path, if no path given does nothing
     * @param optPath Path to descend by, optional for usability.
     * @return `true` if path successfully resolved, `false` otherwise
     */
    bool Importer::descendByPath(const ast::SimplePath::Opt & optPath) {
        if (optPath.none()) {
            return true;
        }

        const auto & res = pathResolver.resolve(
            _importModule, Namespace::Type, optPath.unwrap(), None, ResMode::Descend
        );

        _importModule = sess->defTable.getModule(res.asModuleDef());

        if (res.ok()) {
            return true;
        }
        return false;
    }

    /**
     * @brief Common entry point for items importation. Processes items from all namespaces, applying rebinding logic.
     * @param defPerNS Single name binding per namespace resolved by path (each might be a FOS or simple definition).
     * @param path Any path-like AST node used in importation
     * @param rebind Option rebinding symbol, such as `use path::to::something as newThing`
     */
    void Importer::import(
        const NameBinding::PerNS & defPerNS,
        const ast::PathInterface & path,
        const Option<Symbol> & rebind
    ) {
        log.dev("Import items into module ", _useDeclModule->toString());
        // TODO: Research cases when no the last segment is used!

        // Use last segment as target
        const auto & lastSegIdent = path.lastSegIdent();
        const auto & segSpan = lastSegIdent.span;

        // Use rebinding name or last segment name
        const auto & segName = rebind.some() ? rebind.unwrap() : lastSegIdent.sym;

        // Go through each namespace `PathResolver` found item with name in.
        defPerNS.each([&](const NameBinding::Opt & maybeDef, Namespace nsKind) {
            if (maybeDef.none()) {
                return;
            }

            const auto & nameBinding = maybeDef.unwrap();
            if (nameBinding.isFOS()) {
                defineFOSImportAlias(Vis::Pub, path.getNodeId(), nameBinding.asFOS(), segName, segSpan);
            } else {
                defineImportAlias(nsKind, path.getNodeId(), Vis::Pub, nameBinding.asDef(), segName, segSpan);
            }
        });
    }

    /**
     * @brief Defines import alias to any non-func definition
     * @param nsKind Namespace where item must be placed (same as for imported item)
     * @param pathNodeId Node id of `use` declaration path (where resolution will be mapped)
     * @param importVis Importation visibility - `(pub) use`
     * @param importDefId Identifier of definition to import
     * @param name Name for imported item (might be a rebinding name)
     * @param span `use` declaration span
     */
    void Importer::defineImportAlias(
        Namespace nsKind,
        ast::NodeId pathNodeId,
        Vis importVis,
        DefId importDefId,
        Symbol name,
        span::Span span
    ) {
        auto aliasDefId = sess->defTable.defineImportAlias(importVis, pathNodeId, importDefId);

        log.dev(
            "Import '",
            name,
            "'",
            importDefId,
            " definition from ",
            nsToString(nsKind),
            " namespace as import alias definition ",
            aliasDefId
        );

        _useDeclModule->tryDefine(nsKind, name, aliasDefId).then([&](const NameBinding & oldName) {
            suggestCannotImport(name, span, oldName, None);
        });
    }

    /**
     * @brief Defines FOS import alias in the `use` declaration module
     * @param importVis Visibility of `use` declaration
     * @param pathNodeId Node id of the path node, i.e. node id of `path::to::something` in `use path::to::something`
     * @param importFosId FOSId of resolve FOS
     * @param name The name of FOS, i.e. base name of function overload set
     * @param span Importation span
     */
    void Importer::defineFOSImportAlias(
        Vis importVis,
        ast::NodeId pathNodeId,
        FOSId importFosId,
        Symbol name,
        span::Span span
    ) {
        log.dev(
            Def::visStr(importVis),
            "use '",
            name,
            "' as ",
            importFosId,
            " FOS"
        );

        // If module uses this name -- report an error
        auto fosSearchRes = _useDeclModule->tryFindFOS(name);

        if (fosSearchRes.err()) {
            const auto & oldDefId = fosSearchRes.unwrapErr();
            log.dev("Tried to redefine FOS '", name, "' with non-FOS definition ", oldDefId);
            suggestCannotImport(name, span, oldDefId, None);
            return;
        }

        // If module has FOS with function base name -- we import overloads to it
        auto fosId = fosSearchRes.unwrap();

        // If module does not have definition with this name -- create new FOS and register it in the module
        if (fosId.none()) {
            fosId = sess->defTable.newEmptyFOS();
            _useDeclModule->tryDefineFOS(name, fosId.unwrap());
        }

        // Note: We update FOS present in `use`-declaration module, not the fos we import
        const auto & redefs = sess->defTable.importFos(importVis, pathNodeId, importFosId, fosId.unwrap());
        if (not redefs.ok()) {
            std::string nounSuffix = redefs.suffixes.size() > 1 ? "s" : "";
            std::string error = log::fmt("Cannot `use` function", nounSuffix, " ");

            size_t index = 0;
            for (const auto & suf : redefs.suffixes) {
                error += log::fmt("'", name, suf, "'");
                if (index < redefs.suffixes.size() - 1) {
                    error += ", ";
                }
            }

            error += log::fmt(" because name", nounSuffix, " conflicting with locally defined function", nounSuffix);

            suggestErrorMsg(error, span);
        }
    }

    // Suggestions //
    void Importer::suggestCannotImport(
        Symbol redefinedName,
        const span::Span & span,
        const NameBinding & prevModDef,
        Symbol::Opt suffix
    ) {
        // Pretty similar to `ModuleTreeBuilder::suggestCannotRedefine`

        if (suffix.some()) {
            redefinedName = redefinedName + suffix.unwrap();
        }

        DefId::Opt prevDefId = None;
        if (prevModDef.isFOS()) {
            prevDefId = sess->defTable.getFOSFirstDef(prevModDef.asFOS());
        } else {
            prevDefId = prevModDef.asDef();
        }

        const auto & prevDefSpan = sess->defTable.getDefNameSpan(prevDefId.unwrap());
        const auto & prevDef = sess->defTable.getDef(prevDefId.unwrap());

        // TODO: Header when suggestion headers will be implemented:
        //  log::fmt("Name '", redefinedName, "' for ", Def::kindStr(as), " is conflicting")

        suggest(std::make_unique<sugg::MsgSpanLinkSugg>(
            log::fmt("Cannot import '", redefinedName, "'"),
            span,
            "Because it is already defined as " + prevDef.kindStr() + " here",
            prevDefSpan,
            sugg::SuggKind::Error
        ));
    }
}
