#ifndef JACY_RESOLVE_IMPORTER_H
#define JACY_RESOLVE_IMPORTER_H

#include "ast/StubVisitor.h"
#include "suggest/SuggInterface.h"
#include "resolve/Definition.h"
#include "resolve/Resolutions.h"
#include "data_types/SuggResult.h"
#include "resolve/PathResolver.h"

namespace jc::resolve {
    /// Note: Non-friendly for multi-threading -- global states `_useDeclModule` and `_importModule`

    class Importer : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        Importer() : StubVisitor("Importer") {}
        ~Importer() override = default;

        dt::SuggResult<dt::none_t> declare(sess::Session::Ptr sess, const ast::Party & party);

        void visit(const ast::UseDecl & useDecl) override;
        void visit(const ast::UseTree & useTree) override;

    private:
        log::Logger log{"importer"};
        sess::Session::Ptr sess;

        // Module where `use` appeared (where to add aliases)
        Module::Ptr _useDeclModule;

        // Module to import from (where to search for items)
        Module::Ptr _importModule;

        // Resolutions //
    private:
        PathResolver pathResolver;

        bool descendByPath(const ast::SimplePath::Opt & optPath);

        void import(
            const NameBinding::PerNS & defPerNS,
            const ast::PathInterface & path,
            const Option<Symbol> & rebind
        );

        /**
         * @brief Defines definition of kind `DefKind::Import`, that is an alias to imported definition
         * @param importVis Visibility of `use ...`
         * @param importDefId DefId of imported item
         */
        DefId defineImportAlias(Vis importVis, DefId importDefId, Symbol name);
    };
}

#endif // JACY_RESOLVE_IMPORTER_H
