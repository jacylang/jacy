#ifndef JACY_RESOLVE_IMPORTER_H
#define JACY_RESOLVE_IMPORTER_H

#include "ast/StubVisitor.h"
#include "message/MessageBuilder.h"
#include "resolve/Definition.h"
#include "resolve/Resolutions.h"
#include "resolve/PathResolver.h"
#include "message/MessageResult.h"

namespace jc::resolve {
    /// Note: Non-friendly for multi-threading -- global states `_useDeclModule` and `_importModule`

    class Importer : public ast::StubVisitor {
    public:
        Importer() : StubVisitor("Importer") {}
        ~Importer() override = default;

        message::MessageResult<dt::none_t> declare(sess::Session::Ptr sess, const ast::Party & party);

        void visit(const ast::UseDecl & useDecl) override;
        void visit(const ast::UseTree & useTree) override;

    private:
        log::Logger log{"importer"};
        sess::Session::Ptr sess;

        Vis useDeclVis;

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
        void defineImportAlias(
            Namespace nsKind,
            ast::NodeId pathNodeId,
            Vis importVis,
            DefId importDefId,
            Symbol name,
            span::Span span
        );

        /**
         * @brief Defines new FOS in the module or updates existent one with imported overloads
         * @param importVis
         * @param importFosId
         * @param name
         * @param span
         */
        void defineFOSImportAlias(
            Vis importVis,
            ast::NodeId pathNodeId,
            FOSId importFosId,
            Symbol name,
            span::Span span
        );

        // Messages //
    private:
        message::MessageHolder msg;

        void reportCannotImport(
            Symbol redefinedName,
            span::Span span,
            const NameBinding & prevModDef,
            Symbol::Opt suffix = None
        );
    };
}

#endif // JACY_RESOLVE_IMPORTER_H
