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
        const auto & useDeclModule = sess->defTable.getUseDeclModule(useDecl.id);

        log.dev("Import `use` with module ", useDeclModule->toString());

        _useDeclModule = useDeclModule;
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
            case ast::UseTree::Kind::All: {
                if (descendByPath(useTree.path)) {
                    _importModule->perNS.each([&](const Module::NSMap & ns, Namespace nsKind) {
                        for (const auto & def : ns) {
                            // Note: for `use a::*` we don't report "redefinition" error

                            if (def.second.isFOS()) {
                                _useDeclModule->tryDefineFOS(def.first, def.second.asFOS());
                            } else {
                                _useDeclModule->tryDefine(nsKind, def.first, def.second.asDef());
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
            case ast::UseTree::Kind::Rebind: {
                auto res = pathResolver.resolve(
                    _importModule, Namespace::Any, useTree.path.unwrap(), dt::None, ResMode::Import
                );
                if (res.ok()) {
                    import(res.asImport(), useTree.path.unwrap(), useTree.expectRebinding().sym);
                }
                break;
            }
        }
    }

    /**
     * @brief Descends to module by path, if no path given does nothing
     * @param optPath
     * @return `true` if successfully resolved path, `false` otherwise
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

        // This is a callback common for FOS and definition redefinitions, used below
        const auto redefinitionCallback = [&](const NameBinding & nameBinding) {
            log.dev("Tried to redefine '", segName, "', old name binding is ", nameBinding);
            if (nameBinding.isFOS()) {
                // TODO!!: Function import is a complex process, see Issue #8
                log::notImplemented("Function overloads importation is incomplete feature, see issue #8 on GitHub");
                return;
            }
            auto oldDefId = nameBinding.asDef();
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
        };

        // Go through each namespace `PathResolver` found item with name in.
        defPerNS.each([&](const NameBinding::Opt & maybeDef, Namespace nsKind) {
            if (maybeDef.none()) {
                return;
            }

            const auto & nameBinding = maybeDef.unwrap();

            if (nameBinding.isFOS()) {
                log.dev(
                    "Import '", segName, "' from ", nsToString(nsKind), " namespace as FOS ",
                    nameBinding.asFOS()
                );
                _useDeclModule->tryDefineFOS(segName, nameBinding.asFOS()).then(redefinitionCallback);
            } else {
                log.dev(
                    "Import '", segName, "' from ", nsToString(nsKind), " namespace as definition ",
                    nameBinding.asDef()
                );
                _useDeclModule->tryDefine(nsKind, segName, nameBinding.asDef()).then(redefinitionCallback);
            }
        });
    }
}
