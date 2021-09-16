#include "resolve/Importer.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> Importer::declare(sess::Session::Ptr sess, const ast::Party & party) {
        this->sess = sess;
        pathResolver.init(sess);

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

        auto res = pathResolver.resolve(_importModule, Namespace::Any, useTree.path, None, ResMode::Import);
        define(res.asImport(), useTree.path, None);
    }

    void Importer::visit(const ast::UseTreeSpecific & useTree) {
        // If path given -- descend to module it points to
        if (useTree.path.some()) {
            auto descendModDefId = pathResolver.resolve(
                _importModule, Namespace::Type, useTree.path.unwrap(), None, ResMode::Descend
            ).asModuleDef();
            _importModule = sess->defTable.getModule(descendModDefId);
        }

        // Here, we resolve specifics relatively to current path
        for (const auto & specific : useTree.specifics) {
            specific.autoAccept(*this);
        }
    }

    void Importer::visit(const ast::UseTreeRebind & useTree) {
        auto res = pathResolver.resolve(_importModule, Namespace::Any, useTree.path, dt::None, ResMode::Import);
        define(res.asImport(), useTree.path, useTree.as.unwrap().sym);
    }

    void Importer::visit(const ast::UseTreeAll & useTree) {
        if (useTree.path.some()) {
            auto descendModDefId = pathResolver.resolve(
                _importModule, Namespace::Type, useTree.path.unwrap(), None, ResMode::Descend
            ).asModuleDef();
            _importModule = sess->defTable.getModule(descendModDefId);
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

    void Importer::define(
        const ResResult::UtterValueT & defPerNS,
        const ast::PathInterface & path,
        const Option<Symbol> & rebind
    ) {
        // TODO: Research cases when no the last segment is used!

        // Use last segment as target
        const auto & lastSegIdent = path.lastSegIdent();
        const auto & segSpan = lastSegIdent.span;

        // Use rebinding name or last segment name
        const auto & segName = rebind.some() ? rebind.unwrap() : lastSegIdent.sym;

        // Go through each namespace `PathResolver` found item with name in.
        defPerNS.each([&](const MultiDef & defs, Namespace nsKind) {
            // Go through each definition (only functions have multiple definitions)
            for (const auto & defId : defs) {
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
            }
        });
    }
}
