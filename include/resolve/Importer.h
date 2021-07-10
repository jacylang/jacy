#ifndef JACY_RESOLVE_IMPORTER_H
#define JACY_RESOLVE_IMPORTER_H

#include "ast/StubVisitor.h"
#include "suggest/SuggInterface.h"

namespace jc::resolve {
    class Importer : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        Importer() : StubVisitor("Importer") {}
        ~Importer() override = default;

        void declare(sess::sess_ptr sess);

        void visit(const ast::UseDecl & useDecl) override;
        void visit(const ast::UseTreeRaw & useTree) override;

    private:
        sess::sess_ptr sess;
        module_ptr module;
    };
}

#endif // JACY_RESOLVE_IMPORTER_H
