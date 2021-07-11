#ifndef JACY_RESOLVE_IMPORTER_H
#define JACY_RESOLVE_IMPORTER_H

#include "ast/StubVisitor.h"
#include "suggest/SuggInterface.h"
#include "resolve/Definition.h"
#include "resolve/ResStorage.h"
#include "data_types/SuggResult.h"

namespace jc::resolve {
    /// Path resolution style
    /// Prefix - Resolve all path segments except last one
    /// Full - Resolve full path
    enum class PathResKind {
        Prefix,
        Full,
    };

    class Importer : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        Importer() : StubVisitor("Importer") {}
        ~Importer() override = default;

        dt::SuggResult<dt::none_t> declare(sess::sess_ptr sess, const ast::Party & party);

        void visit(const ast::UseDecl & useDecl) override;
        void visit(const ast::UseTreeRaw & useTree) override;
        void visit(const ast::UseTreeSpecific & useTree) override;

    private:
        common::Logger log{"importer"};
        sess::sess_ptr sess;

        // Module where `use` appeared
        module_ptr _declModule;

        // Module that `use` now in
        module_ptr _importModule;

        // Resolutions //
    private:
        void resolvePath(const ast::SimplePath & path, const std::function<void()> & targetCb);
    };
}

#endif // JACY_RESOLVE_IMPORTER_H
