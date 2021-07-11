#ifndef JACY_RESOLVE_IMPORTER_H
#define JACY_RESOLVE_IMPORTER_H

#include "ast/StubVisitor.h"
#include "suggest/SuggInterface.h"
#include "resolve/Definition.h"
#include "resolve/ResStorage.h"
#include "data_types/SuggResult.h"

namespace jc::resolve {
    using DefPerNS = PerNS<opt_def_id>;
    struct PathResult {
        DefPerNS defPerNs;
        ast::simple_path_seg_ptr lastSeg;
    };

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

        dt::SuggResult<dt::none_t> declare(sess::sess_ptr sess, const ast::Party & party);

        void visit(const ast::UseDecl & useDecl) override;
        void visit(const ast::UseTreeRaw & useTree) override;
        void visit(const ast::UseTreeSpecific & useTree) override;
        void visit(const ast::UseTreeRebind & useTree) override;
        void visit(const ast::UseTreeAll & useTree) override;

    private:
        common::Logger log{"importer"};
        sess::sess_ptr sess;

        // Module where `use` appeared
        module_ptr _useDeclModule;

        // Module that `use` now in
        module_ptr _importModule;

        // Resolutions //
    private:
        PathResult resolvePath(PathResKind resKind, const ast::SimplePath & path);
        void define(PathResult && pathResult, const dt::Option<std::string> & rebind);
    };
}

#endif // JACY_RESOLVE_IMPORTER_H
