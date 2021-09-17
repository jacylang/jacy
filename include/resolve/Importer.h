#ifndef JACY_RESOLVE_IMPORTER_H
#define JACY_RESOLVE_IMPORTER_H

#include "ast/StubVisitor.h"
#include "suggest/SuggInterface.h"
#include "resolve/Definition.h"
#include "resolve/Resolutions.h"
#include "data_types/SuggResult.h"
#include "resolve/PathResolver.h"

namespace jc::resolve {
    /// Path resolution style
    /// `Prefix`
    ///   - Resolve all path segments except last one
    ///   - used for `use a::b` and `a::b as c`
    /// `Full`
    ///   - Resolve full path searching for each segment in Type namespace
    ///   - used for `use a::{}` and `use a::*`
    enum class PathResKind {
        Prefix,
        Full,
    };

    class Importer : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        Importer() : StubVisitor("Importer") {}
        ~Importer() override = default;

        dt::SuggResult<dt::none_t> declare(sess::Session::Ptr sess, const ast::Party & party);

        void visit(const ast::UseDecl & useDecl) override;
        void visit(const ast::UseTreeRaw & useTree) override;
        void visit(const ast::UseTreeSpecific & useTree) override;
        void visit(const ast::UseTreeRebind & useTree) override;
        void visit(const ast::UseTreeAll & useTree) override;

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

        void define(
            const NameBinding::PerNS & defPerNS,
            const ast::PathInterface & path,
            const Option<Symbol> & rebind
        );
    };
}

#endif // JACY_RESOLVE_IMPORTER_H
